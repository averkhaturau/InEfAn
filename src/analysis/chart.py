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
            hist_delta = datetime.timedelta(minutes=1)
            start = min(key_press_events[0], mouse_click_events[0], mouse_other_events[0]).replace(#minute=0,
                second=0,microsecond=0)
            finish = max(key_press_events[-1], mouse_click_events[-1], mouse_other_events[-1]).replace(#minute=0,
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

            plt.title("Average Input Events per minute")
            keypress_bar = plt.bar(ind, keypress_hist, width, color='r', edgecolor='b')
            clicks_bar = plt.bar(ind_m, m_click_hist, width, color='g', edgecolor='b')
            moves_bar = plt.bar(ind_m, m_other_hist, width, color='y', edgecolor='b', bottom=m_click_hist)

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
                            arrowprops=dict(facecolor='grey',arrowstyle='->'),
                            xytext=(ann_index, this_height + random.random()*( max_height-this_height)))
                ann_index += 1

            for inefan_exits in log_parse.inefan_exit_events:
                exit_ind = timedelta2Minutes(inefan_exits - start) / timedelta2Minutes(hist_delta)
                height = random.random() * 0.3 * max_height
                plt.annotate("Here InEfAn log interrupts...", xy=(exit_ind, 1),
                    arrowprops=dict(arrowstyle='->'), xytext=(exit_ind+1, height))

            # TODO: find where 3 or more hours of silence
            #plt.annotate("HERE I FELT TIRED\nAND WENT TO BED",
            #    xy=(13, 100), arrowprops=dict(arrowstyle='->'), xytext=(15,
            #    1000))

            plt.ylabel("Events per minute")
            plt.xticks(ind, x_axis)
            plt.xticks(rotation='vertical')
            plt.legend((keypress_bar[0], clicks_bar[0], moves_bar[0]), ("Key Press events", "Mouse Click events", "Mouse Moves and Scrolls"))


            fig = plt.gcf()
            fig.patch.set_facecolor('black')
            axes = plt.gca()
            axes.patch.set_facecolor('black')
            fig.set_size_inches(19.2,10.8, forward=True)
            fig.savefig("res/chart.png", dpi=100, transparent=True)
#            plt.show()

        #except:
        #    print("Unexpected error {}".format(sys.exc_info()[0]))

