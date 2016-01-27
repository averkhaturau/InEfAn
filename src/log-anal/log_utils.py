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
    while curr < end:
        yield curr
        curr += delta


def group_by_intervals(events, start, interval):
    period_start = start
    period_end = period_start + interval
    hist = [[]] # array of arrays
    for evt in events:
        if evt < period_end:
            hist[-1].append(evt)
        else:
            hist.append([evt])
            period_start = period_end
            period_end = period_start + interval

    return hist


def norm_events_stat_to_hist(events, start, interval):
    return list(map(lambda a: len(a), group_by_intervals(events, start, interval)))
