import matplotlib.pyplot as plt
import matplotlib.dates as mdates
import numpy as np
import datetime
import random

#local
import log_parse
from utils import *
    

def log_plot(key_press_events, mouse_click_events, mouse_other_events, foreground_windows):
    if not (key_press_events and mouse_click_events and mouse_other_events):
        print("Not enough statistics for the chart :(")
        return

    with plt.xkcd():
        #try:
            hist_delta = datetime.timedelta(hours=1)
            start = min(key_press_events[0], mouse_click_events[0], mouse_other_events[0]).replace(minute=0,
                second=0,microsecond=0)
            finish = max(key_press_events[-1], mouse_click_events[-1], mouse_other_events[-1]).replace(minute=0,
                second=0,microsecond=0) + hist_delta

            keypress_hist = norm_events_stat_to_hist(key_press_events, start, finish, hist_delta)
            m_click_hist = norm_events_stat_to_hist(mouse_click_events, start, finish, hist_delta)
            m_other_hist = norm_events_stat_to_hist(mouse_other_events, start, finish, hist_delta)

            #print(keypress_hist)
            #print(m_click_hist)
            #print(m_other_hist)

            #l = plt.plot(keypress_hist, 'r', linewidth=2)
            x_axis = list(map(lambda dt: dt.strftime("%d/%m %H:%M"), (perdelta(start, finish, hist_delta))))
            #print (x_axis)

            width = .35
            ind = range(len(x_axis))
            ind_m = [x + width for x in ind]
            max_height = max(keypress_hist + m_other_hist)

            plt.title("Average Input Events per hour")
            keypress_bar = plt.bar(ind, keypress_hist, width, color='r')
            clicks_bar = plt.bar(ind_m, m_click_hist, width, color='g')
            moves_bar = plt.bar(ind_m, m_other_hist, width, color='y', bottom=m_click_hist)

            annotations = apps_usage_stat(foreground_windows, start, hist_delta)
            #print(annotations)
            ann_index = 0
            for ann in annotations:
                if ann:
                    ann_text = "{} - {}%".format(ann[0][0], ann[0][1])
                    for app_usage in ann[1:]:
                        if app_usage[1]:
                            ann_text += "\n{} - {}%".format(app_usage[0], app_usage[1])
                    this_height = max(keypress_hist[ann_index],m_click_hist[ann_index] + m_other_hist[ann_index])
                    if this_height:
                        plt.annotate(ann_text,
                            xy=(ann_index + width, this_height),
                            arrowprops=dict(arrowstyle='->'),
                            xytext=(ann_index, this_height + random.random() * (max_height - this_height)))
                ann_index += 1

            for inefan_exits in log_parse.inefan_exit_events:
                exit_ind = timedelta2Minutes(inefan_exits - start) / timedelta2Minutes(hist_delta)
                height = random.random() * 0.3 * max_height
                plt.annotate("Here InEfAn log interrupts...", xy=(exit_ind, 1),
                    arrowprops=dict(arrowstyle='->'), xytext=(exit_ind + 1, height))

            plt.ylabel("Events per minute")
            plt.xticks(ind, x_axis)
            plt.xticks(rotation='vertical')
            plt.legend((keypress_bar[0], clicks_bar[0], moves_bar[0]), ("Key Press events", "Mouse Click events", "Mouse Moves and Scrolls"))

            plt.show()
        #except:
        #    print("Unexpected error {}".format(sys.exc_info()[0]))



def plot_transitions(k2m,m2k):


    k2m_starts=[]
    k2m_stops=[]
    for intrvl in k2m:
        k2m_starts.append(intrvl[0])
        k2m_stops.append(intrvl[1])
    plt.hlines(np.repeat(1,len(k2m)),#range(1,len(k2m)+1),
        k2m_starts, k2m_stops, 'y', lw=10)

    m2k_starts=[]
    m2k_stops=[]
    for intrvl in m2k:
        m2k_starts.append(intrvl[0])
        m2k_stops.append(intrvl[1])
    plt.hlines(np.repeat(1,len(m2k)),# range(2,len(m2k)+2),
        m2k_starts, m2k_stops, 'b', lw=10)

    #Setup the plot
    ax = plt.gca()
    ax.xaxis_date()
    myFmt = mdates.DateFormatter('%H:%M')
    ax.xaxis.set_major_formatter(myFmt)
    ax.xaxis.set_major_locator(mdates.SecondLocator(interval=60))

    #To adjust the xlimits a timedelta is needed.
    #delta = (stop.max() - start.min())/10

    #plt.yticks(y[unique_idx], captions)
    plt.ylim(0,2)#max(len(m2k),len(k2m)+2))
    plt.xlim(min(k2m_starts[0], m2k_starts[0]), max(m2k_stops[-1], k2m_stops[-1]))
    plt.xlabel('Time')
    plt.show()