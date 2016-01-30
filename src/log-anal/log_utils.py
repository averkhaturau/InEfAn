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


def timdelta2Minutes(td):
    return td.days * 24. * 60 + td.seconds / 60.


def calc_total_trastition_time(transition_events):
    return sum(map(lambda e1_e2: e1_e2[1] - e1_e2[0], transition_events), datetime.timedelta())


def calc_mean_trastition_time(transition_events):
    if len(transition_events) > 0:
        return calc_total_trastition_time(transition_events) / len(transition_events)


def calc_typing_speed(num_keypresses, timespan):
    minutes = timdelta2Minutes(timespan)
    if minutes > 0:
        return num_keypresses / timdelta2Minutes(timespan)


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

    desired_out_length = timdelta2Minutes(finish - start + interval) / timdelta2Minutes(interval)
    while len(hist) < desired_out_length:
        hist.append([])

    return hist


def norm_events_stat_to_hist(events, start, finish, interval):
    return list(map(lambda a: len(a), group_by_intervals(events, start, finish, interval)))


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
        result.append(list(map(lambda app_time: (app_time[0],int(timdelta2Minutes(app_time[1]) * 100 / timdelta2Minutes(hist_delta))),
            sorted(filter(lambda item: item[0], period.items()), key=lambda x: -x[1]))))

    return result
