import matplotlib.pyplot as plt
import matplotlib.dates as mdates
import datetime

def make_histogram(events, interval):
    period_start = events[0]
    period_end = period_start + interval
    hist = [[events[0]]] # array of arrays
    for evt in events[1:]:
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


def log_plot(key_press_events):
    hist_delta = datetime.timedelta(hours=1)
    keypress_hist = map(lambda a: len(a), make_histogram(key_press_events, hist_delta))
    # print(map(lambda a: len(a), keypress_hist))
    #n, bins, patches = plt.hist(keypress_hist, normed=1, facecolor='green')
    l = plt.plot(keypress_hist, 'r--', linewidth=1)
    x_axis = map(lambda dt: unicode(dt),(perdelta(key_press_events[0], key_press_events[-1], hist_delta)))
    print (x_axis)
    #plt.axis(x_axis)
    plt.show()
