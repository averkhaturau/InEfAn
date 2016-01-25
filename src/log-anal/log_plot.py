import matplotlib.pyplot as plt
import matplotlib.dates as mdates
import numpy as np
import datetime
import log_parse

def make_histogram(events, start, interval):
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


def perdelta(start, end, delta):
    curr = start
    while curr < end:
        yield curr
        curr += delta

def norm_events_stat_to_hist(events, start, interval):
    return list(map(lambda a: len(a), make_histogram(events, start, interval)))
    

def log_plot(key_press_events, mouse_click_events, mouse_other_events):

    hist_delta = datetime.timedelta(hours=1)
    start = min(key_press_events[0], mouse_click_events[0], mouse_other_events[0]).replace(minute=0,second=0,microsecond=0)

    keypress_hist = norm_events_stat_to_hist(key_press_events, start, hist_delta)
    m_click_hist = norm_events_stat_to_hist(mouse_click_events, start, hist_delta)
    m_other_hist = norm_events_stat_to_hist(mouse_other_events, start, hist_delta)

    print(keypress_hist)
    print(m_click_hist)
    print(m_other_hist)

    #l = plt.plot(keypress_hist, 'r', linewidth=2)
    x_axis = list(map(lambda dt: dt.strftime("%d/%m %H:%M"), (perdelta(start, key_press_events[-1], hist_delta))))
    #print (x_axis)

    ind = range(len(x_axis))
    width = .4

    plt.title("Average Input Events per hour")
    keypress_bar = plt.bar(ind, keypress_hist, width, color='r')
    clicks_bar = plt.bar(ind, m_click_hist, width, color='m', bottom=keypress_hist)
    moves_bar = plt.bar(ind, m_other_hist, width, color='y', bottom=m_click_hist)

    plt.ylabel("Events per minute")
    plt.xticks(ind, x_axis)
    plt.xticks(rotation='vertical')
    plt.legend((keypress_bar[0], clicks_bar[0], moves_bar[0]), ("Key Press events", "Mouse Click events", "Mouse Moves and Scrolls"))

    plt.show()
