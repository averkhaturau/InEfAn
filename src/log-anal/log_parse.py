import sys
import datetime
import re
from log_utils import *

####################################################################
#  Global variables
parsing_header = False

is_using_mouse = False
is_using_keyboard = False

foreground_windows = []

activity_periods = []
                                            
# to estimate typing speed
key_press_events = []
# to counterpose mouse events to key presses
mouse_click_events = []
mouse_other_events = []

inactivity_interval = datetime.timedelta(seconds=4) # if no events during this time span, consider user inactive
typing_keypresses_intervals = []

# filtered
unique_input_events = []

mouse_to_kb = []
kb_to_mouse = []

is_header_info_shown = False

analysis_begin = datetime.datetime(1917,11,7)
analysis_end = datetime.datetime.now()

####################################################################


# treats header and event line different
def handle_log_line(line):
    global parsing_header
    if parsing_header:
        if line.startswith("==="):
            parsing_header = False
        else:
            get_data_from_header(line)

    else:
        if line.startswith("==="):
            parsing_header = True
        else:
            parse_log_line(line)


# get system info from header line
def get_data_from_header(line):
    global is_header_info_shown
    if is_header_info_shown: return
    m = re.search("(?<=running on)[ \\t]+(.*)[ \\t]+powered by[ \\t]+(.*)", line)
    if m:
        global pc_type,os_version
        pc_type = m.group(1)
        os_version = m.group(2)
        print("Running on " + pc_type + " powered by " + os_version)
        is_header_info_shown = True


# read simple event line
def parse_log_line(line):

    line_word = [w for w in re.split(" |\t|\n|\r", line) if w]
    if len(line_word) < 3:
        return
    event_time = datetime.datetime.strptime(line_word[0] + " " + line_word[1], "%Y-%m-%d %H:%M:%S.%f")
    # print(event_time, line_word[2:])

    global current_window, analysis_begin, analysis_end

    if not (analysis_begin < event_time < analysis_end):
        return

    if line_word[2] == "mouse":
        if line_word[-1] == "up" or line_word[-1] == "finished":
            #print("Mouse using stopped at " + str(event_time))
            if (not unique_input_events or unique_input_events[-1][0] != "mouse stopped"):
                unique_input_events.append(("mouse stopped", event_time))
            else:
                unique_input_events[-1] = ("mouse stopped", event_time)
        elif line_word[-1] == "down" or line_word[-1] == "started":
            #print("Mouse using started at " + unicode(event_time))
            if (not unique_input_events or unique_input_events[-1][0] != "mouse started"):
                unique_input_events.append(("mouse started", event_time))
            if line_word[-1] == "down":
                mouse_click_events.append(event_time)
            else: 
                mouse_other_events.append(event_time)
        else:
            print("Mouse single event at " + str(event_time))
            mouse_other_events.append(event_time)

        user_is_active_at(event_time)

    elif line_word[2] == "keyboard":
        if line_word[-1] == "up":
            #print("Keyboard using stopped at " + unicode(event_time))
            if (not unique_input_events or unique_input_events[-1][0] != "keyboard stopped"):
                unique_input_events.append(("keyboard stopped", event_time))
            else:
                unique_input_events[-1] = ("keyboard stopped", event_time)
            if line_word[3] in ("letter", "digit", "SPACEBAR", "-","+",",",".", "\\"):
                key_press_events.append(event_time)
        elif line_word[-1] == "down":
            #print("Keyboard using started at " + unicode(event_time))
            if (not unique_input_events or unique_input_events[-1][0] != "keyboard started"):
                unique_input_events.append(("keyboard started", event_time))
        else:
            print("Keyboard single event at " + str(event_time))

        user_is_active_at(event_time)

    else:
        # system event
        evt_text = " ".join(line_word[2:])
        #print((evt_text + " at " +
        #unicode(event_time)).encode(sys.stdout.encoding, errors='replace'))

        m = re.search('Foreground window title is "(.*)" from process name "(.*)" running from file "(.*)"', evt_text)
        if m:
            current_window = {\
                "title": m.group(1),\
                "procname":m.group(2),\
                "filename":m.group(3)\
            }
        else:
            current_window = None # FIXME
        foreground_windows.append((event_time, current_window))


# determine user activity periods
def user_is_active_at(t):
    if len(activity_periods) == 0 or (t - activity_periods[-1]) > inactivity_interval:
        activity_periods.append(t)
        activity_periods.append(t)
    else:
        activity_periods[-1] = t


def print_characteristics():
    global mean_typing_speed, mouse_to_kb, kb_to_mouse
    activity_time = datetime.timedelta()
    it = iter(activity_periods)
    for period_start in it:
        period_end = next(it)
        if period_end - period_start < inactivity_interval:
            continue

        activity_time += period_end - period_start

        # calculate typing speed
        key_presses_in_period = tuple(filter(lambda press_time: period_start <= press_time <= period_end, key_press_events))
        num_keypresses = len(key_presses_in_period)
        if num_keypresses < 3:
            continue
        typing_interval = key_presses_in_period[-1] - key_presses_in_period[0]
        typing_keypresses_intervals.append((num_keypresses, typing_interval))
        # print("Typing speed is {} at {}".format(calc_typing_speed(num_keypresses, typing_interval), period_end))
        # print(period_start, period_end, num_keypresses, typing_speed)

        # calcuate mouse-to-keyboard switch time
        events_scope = list(filter(lambda eType_eTime: period_start <= eType_eTime[1] <= period_end, unique_input_events))
        for (e1,e2) in pairwise(events_scope):
            if e1[0] == "mouse stopped" and e2[0] == "keyboard started":
                mouse_to_kb.append((e1[1],e2[1]))
            elif e1[0] == "keyboard stopped" and e2[0] == "mouse started":
                kb_to_mouse.append((e1[1],e2[1]))
        

    if len(unique_input_events) < 5 or len(activity_periods) < 1:
        print("Not enough observation, please gather more statistics")
        return

    mean_typing_speed = calc_typing_speed(\
        sum(map(lambda s_i: s_i[0], typing_keypresses_intervals)), \
        sum(map(lambda s_i: s_i[1], typing_keypresses_intervals), datetime.timedelta()))
    print("Mean Typing speed is {}".format(mean_typing_speed))

    mean_mouse_to_kb = calc_mean_trastition_time(mouse_to_kb)
    print("Mean time to transit hand from mouse to keyboard = {}".format(mean_mouse_to_kb))
    mean_kb_to_mouse = calc_mean_trastition_time(kb_to_mouse)
    print("Mean time to transit hand from keyboard to mouse = {}".format(mean_kb_to_mouse))

    observation_period = unique_input_events[-1][1] - unique_input_events[0][1]
    if timdelta2Minutes(observation_period) < 1:
        print("Observation time is not enough for statistics...")

    print("You were active {:1.1f} minutes during {:1.1f} observed, which is {:1.1f}%".format(\
        timdelta2Minutes(activity_time), timdelta2Minutes(observation_period), 100 * timdelta2Minutes(activity_time) / timdelta2Minutes(observation_period)))

    if timdelta2Minutes(activity_time) < 1:
        print("Active interval was less then a minute, which is not enough for statistics")
    hand_moves_per_hour = (len(mouse_to_kb) + len(kb_to_mouse)) * 60. / timdelta2Minutes(activity_time)
    hand_moving_time = calc_total_trastition_time(mouse_to_kb + kb_to_mouse)
    hand_moving_percents = timdelta2Minutes(hand_moving_time) * 100 / timdelta2Minutes(activity_time)
    print("You have moved your hand from mouse to keyboard {} times and {} times back, you do it average {:1.1f} times per hour and this tooks you {:3.1f}% of your active time".format(\
        len(mouse_to_kb), len(kb_to_mouse), (len(mouse_to_kb) + len(kb_to_mouse)) * 60 / timdelta2Minutes(activity_time), hand_moving_percents))


