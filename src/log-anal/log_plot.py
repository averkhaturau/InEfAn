import matplotlib.pyplot as plt
import matplotlib.dates as mdates
import numpy as np
import datetime
import log_parse
from log_utils import *
    

def log_plot(key_press_events, mouse_click_events, mouse_other_events):

    hist_delta = datetime.timedelta(minutes=1)
    start = min(key_press_events[0], mouse_click_events[0], mouse_other_events[0]).replace(second=0,microsecond=0)

    keypress_hist = norm_events_stat_to_hist(key_press_events, start, hist_delta)
    m_click_hist = norm_events_stat_to_hist(mouse_click_events, start, hist_delta)
    m_other_hist = norm_events_stat_to_hist(mouse_other_events, start, hist_delta)

    #print(keypress_hist)
    #print(m_click_hist)
    #print(m_other_hist)

    #l = plt.plot(keypress_hist, 'r', linewidth=2)
    x_axis = list(map(lambda dt: dt.strftime("%d/%m %H:%M"), (perdelta(start, key_press_events[-1], hist_delta))))
    #print (x_axis)

    width = .25
    ind = range(len(x_axis))
    ind_m = [x + width for x in ind]

    plt.title("Average Input Events per hour")
    keypress_bar = plt.bar(ind, keypress_hist, width, color='r')
    clicks_bar = plt.bar(ind_m, m_click_hist, width, color='g')
    moves_bar = plt.bar(ind_m, m_other_hist, width, color='y', bottom=m_click_hist)

    plt.ylabel("Events per minute")
    plt.xticks(ind, x_axis)
    plt.xticks(rotation='vertical')
    plt.legend((keypress_bar[0], clicks_bar[0], moves_bar[0]), ("Key Press events", "Mouse Click events", "Mouse Moves and Scrolls"))

    plt.show()
