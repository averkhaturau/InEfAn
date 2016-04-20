import sys
import datetime
from itertools import *


def pairwise(iterable):                    
    "s -> (s0,s1), (s1,s2), (s2, s3), ..." 
    a, b = tee(iterable)
    try:
        next(b)
    except StopIteration:
        pass
    return zip(a, b)


def timedelta2Minutes(td):
    return td.days * 24. * 60 + td.seconds / 60. + td.microseconds / 60000000


def calc_total_trastition_time(transition_events):
    return sum(map(lambda e1_e2: e1_e2[1] - e1_e2[0], transition_events), datetime.timedelta())


def calc_mean_trastition_time(transition_events):
    if len(transition_events) > 0:
        return calc_total_trastition_time(transition_events) / len(transition_events)


def calc_typing_speed(num_keypresses, timespan):
    minutes = timedelta2Minutes(timespan)
    if minutes > 0:
        return num_keypresses / minutes
    return 0


def perdelta(start, end, delta):
    curr = start
    while curr <= end:
        yield curr
        curr += delta


def group_by_intervals(events, start, finish, interval):
    period_start, period_end = start, start + interval
    hist = [[]] # array of arrays
    for evt in events:
        if period_start <= evt < period_end:
            hist[-1].append(evt)
        else:
            while evt > period_end:
                period_start, period_end = period_end, period_end + interval
                hist.append([])
            hist[-1].append(evt)

    return hist


def group_intervals_by_intervals(events, start, finish, interval):
    period_start, period_end = start, start + interval
    hist = [[]] # array of arrays
    for evt in events:
        if period_start <= evt[0] < period_end:
            hist[-1].append(evt)
        else:
            while evt[0] > period_end:
                period_start, period_end = period_end, period_end + interval
                hist.append([])
            hist[-1].append(evt)

    return hist

def norm_events_stat_to_hist(events, start, finish, interval):
    return list(map(lambda a: len(a), group_by_intervals(events, start, finish, interval)))


def intervals_to_hist(events, start, finish, interval):
    return list(map(lambda a: sum([i[1]-i[0] for i in a], datetime.timedelta()), group_intervals_by_intervals(events, start, finish, interval)))


def apps_usage_stat(foreground_windows, start, hist_delta):
    period_stats = [{}]
    end = start + hist_delta
    prev_proc = None
    for time_app in foreground_windows:
        if not time_app[1]:
            time_app = (time_app[0], {"procname":""})
        if start <= time_app[0] < end:
            if prev_proc:
                new_usage_time = (time_app[0] - prev_proc[0])
                if period_stats[-1] and prev_proc[1]["procname"] in period_stats[-1]:
                    new_usage_time += period_stats[-1][prev_proc[1]["procname"]]
                period_stats[-1].update({prev_proc[1]["procname"]: new_usage_time})
        else:
            while time_app[0] >= end:
                start,end = end, end + hist_delta
                if prev_proc:
                    if period_stats[-1] and prev_proc[1]["procname"] in period_stats[-1]:
                        new_usage_time = min(period_stats[-1][prev_proc[1]["procname"]] + (start - prev_proc[0]), hist_delta)
                        period_stats[-1].update({prev_proc[1]["procname"]: new_usage_time})
                    period_stats.append({prev_proc[1]["procname"]: min(time_app[0],end) - start})

        prev_proc = time_app

    result = []
    for period in period_stats:
        result.append(list(map(lambda app_time: (app_time[0],int(timedelta2Minutes(app_time[1]) * 100 / timedelta2Minutes(hist_delta))),
            sorted(filter(lambda item: item[0], period.items()), key=lambda x: -x[1])[:3])))

    return result


def flattern(list_of_lists):
    return list(chain.from_iterable(list_of_lists))


def app_intervals(fw):
    intervals_with_app = []
    for app in fw:
        if intervals_with_app and intervals_with_app[-1][0] != app[1]["procname"]:
            intervals_with_app[-1][1].append(app[0])
            intervals_with_app.append([app[1]["procname"], [app[0]]])
        elif not intervals_with_app:
            intervals_with_app.append([app[1]["procname"], [app[0]]])
    by_app = {}
    for a in intervals_with_app:
        if not a[0] in by_app:
            by_app.update({a[0]:[]})
        by_app[a[0]].append(a[1])
    return by_app


def sum_intervals(list_of_intervals):
    intervals_length = [i[1]-i[0] for i in list_of_intervals if i and len(i)==2]
    return sum(intervals_length, datetime.timedelta())


def cross_interval(i1, i2):
    if i1[0] <= i2[1] and i1[1] >= i2[0]:
        return [max(i1[0], i2[0]), min(i1[1], i2[1])]


def cross_intervals(i1_list, i2_list, get_comparable_fn, to_orig_fn):
    iter1 = iter(i1_list)
    iter2 = iter(i2_list)

    result = []

    try:
        val1 = next(iter1)
        val_orig = next(iter2)
        val2 = get_comparable_fn(val_orig)

        while val1[1] < val2[0]:
            val1 = next(iter1)
        while val2[1] < val1[0]:
            val_orig = next(iter2)
            val2 = get_comparable_fn(val_orig)

        while iter1 and iter2:
            x_intrvl = cross_interval(val1, val2)
            if x_intrvl:
                result.append(to_orig_fn(val_orig, x_intrvl))
            if val1[1] < val2[1]:
                val1 = next(iter1)
            else:
                val_orig = next(iter2)
                val2 = get_comparable_fn(val_orig)
    except StopIteration:
        pass
    except IndexError:
        pass
    return result

