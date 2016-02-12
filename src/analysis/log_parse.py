import sys
import datetime
import re

from utils import *

####################################################################
#  Global variables
parsing_header = False

is_using_mouse = False
is_using_keyboard = False

foreground_windows = []

activity_periods = []
                                            
# to estimate typing speed
key_press_event_groups = [[]]
key_press_events = [] # output to graph

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

inefan_exit_events = []
last_parsed_time = None

keys_and_scrolls = []

transition_to_scrolling = []

is_ctrl_key_down = False
is_alt_key_down = False

isolated_mouse_events = [[]]
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
            global last_parsed_time
            if last_parsed_time:
                inefan_exit_events.append(last_parsed_time)
                last_parsed_time = None
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
        global last_parsed_time
        if last_parsed_time and line == "Disabling logging...":
            inefan_exit_events.append(last_parsed_time)
            last_parsed_time = None
        return
    event_time = datetime.datetime.strptime(line_word[0] + " " + line_word[1], "%Y-%m-%d %H:%M:%S.%f")
    last_parsed_time = event_time
    # print(event_time, line_word[2:])

    global current_window, analysis_begin, analysis_end, is_ctrl_key_down, is_alt_key_down, key_press_event_groups

    if not (analysis_begin < event_time < analysis_end):
        return

    if line_word[2] == "mouse":
        if line_word[3] in ("wheel","h-wheel"):
            keys_and_scrolls.append(("scroll "+line_word[-1], event_time))

        if line_word[-1] == "up" or line_word[-1] == "finished":
            #print("Mouse using stopped at " + str(event_time))
            if (not unique_input_events or unique_input_events[-1][0] != "mouse stopped"):
                unique_input_events.append(("mouse stopped", event_time))
            else:
                unique_input_events[-1] = ("mouse stopped", event_time)
        elif line_word[-1] == "down" or line_word[-1] == "started":
            if key_press_event_groups[-1]:
                key_press_event_groups.append([])
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
        keys_and_scrolls.append(("key "+line_word[-1], event_time))
        if line_word[-1] == "up":
            if len(line_word)>4:
                if line_word[4] == "CONTROL":
                    is_ctrl_key_down = False
                elif line_word[4] == "MENU":
                    is_alt_key_down = False

            #print("Keyboard using stopped at " + unicode(event_time))
            if (not unique_input_events or unique_input_events[-1][0] != "keyboard stopped"):
                unique_input_events.append(("keyboard stopped", event_time))
            else:
                unique_input_events[-1] = ("keyboard stopped", event_time)
            if line_word[3] in ("letter", "digit", "SPACEBAR", "-","+",",",".", "\\") and not (is_ctrl_key_down or is_alt_key_down):
                key_press_event_groups[-1].append(event_time)
        elif line_word[-1] == "down":
            if len(line_word)>4:
                if line_word[4] == "CONTROL":
                    is_ctrl_key_down = True
                elif line_word[4] == "MENU":
                    is_alt_key_down = True

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
    global mean_typing_speed, mouse_to_kb, kb_to_mouse, transition_to_scrolling, typing_keypresses_intervals, key_press_events, isolated_mouse_events
    activity_time = datetime.timedelta()
    it = iter(activity_periods)
    for period_start in it:
        period_end = next(it)
        if period_end - period_start < inactivity_interval:
            continue

        activity_time += period_end - period_start

        # calculate typing speed
        key_presses_in_period = []
        for group in key_press_event_groups:
            filtered_group = list(filter(lambda press_time: period_start <= press_time <= period_end, group))
            if len(filtered_group) > 3:
                key_presses_in_period.append(filtered_group)
	
        if key_presses_in_period:
            key_press_events += flattern(key_presses_in_period)

        typing_keypresses_intervals += list(map(lambda g: (len(g), g[-1]-g[0]), key_presses_in_period))

        # print("Typing speed is {} at {}".format(calc_typing_speed(num_keypresses, typing_interval), period_end))
        # print(period_start, period_end, num_keypresses, typing_speed)

        events_scope = list(filter(lambda eType_eTime: period_start <= eType_eTime[1] <= period_end, unique_input_events))

        # calcuate mouse-to-keyboard switch time
        for (e1,e2) in pairwise(events_scope):
            if e1[0] == "mouse stopped" and e2[0] == "keyboard started":
                mouse_to_kb.append((e1[1],e2[1]))
            elif e1[0] == "keyboard stopped" and e2[0] == "mouse started":
                kb_to_mouse.append((e1[1],e2[1]))

        # calc isolated mouse usages, e.g. time on mouse in activity periods, between key presses
        gathering = False
        for evt in events_scope:
            if evt[0] == "keyboard stopped" and not gathering:
                gathering = True
                if isolated_mouse_events[-1]:
                    isolated_mouse_events.append([])
            elif evt[0] == "keyboard started":
                gathering = False
            else:
                if gathering:
                     isolated_mouse_events[-1].append(evt[1])
        if gathering and isolated_mouse_events[-1]:
            isolated_mouse_events.pop()

        # calc hand-transition time for scrolling only
        transition_to_scrolling_in_period = list(filter(
            lambda e1_e2: e1_e2[0][0] == "key up" and e1_e2[1][0] == "scroll started",
            pairwise(list(filter(lambda evt: period_start <= evt[1] <= period_end, keys_and_scrolls)))))
        transition_to_scrolling += map(lambda evts: (evts[0][1], evts[1][1]), transition_to_scrolling_in_period)

    #print("transition_to_scrolling = {}".format(transition_to_scrolling))

    if len(unique_input_events) < 5 or len(activity_periods) < 1:
        print("Not enough observation, please gather more statistics.")
        return

    mean_typing_speed = calc_typing_speed(\
        sum(map(lambda s_i: s_i[0], typing_keypresses_intervals)), \
        sum(map(lambda s_i: s_i[1], typing_keypresses_intervals), datetime.timedelta()))

    if not mean_typing_speed:
        print("Not enough observation, please gather more statistics.")
        return

    print("Mean Typing speed is {}".format(mean_typing_speed))

    mean_mouse_to_kb = calc_mean_trastition_time(mouse_to_kb)
    print("Mean time to transit hand from mouse to keyboard = {}.".format(mean_mouse_to_kb))
    mean_kb_to_mouse = calc_mean_trastition_time(kb_to_mouse)
    print("Mean time to transit hand from keyboard to mouse = {}.".format(mean_kb_to_mouse))
    hand_moving_time = calc_total_trastition_time(mouse_to_kb + kb_to_mouse)
    print("During the observation you moved your hand total {}.".format(hand_moving_time))

    observation_period = unique_input_events[-1][1] - unique_input_events[0][1]
    # extrapolate statistics to 1 year
    one_year_rate = 365.25*24*60 / timedelta2Minutes(observation_period)
    print(("For 1 year you would spend {:1.0f} hours of you life to move you hand to mouse and back," +
        " if you use you PC like you do during the observed time.")
        .format(timedelta2Minutes(hand_moving_time) * one_year_rate / 60))


    if timedelta2Minutes(observation_period) < 1:
        print("Observation time is not enough for statistics...")

    print("You were active {:1.1f} minutes during {:1.1f} observed, which is {:1.1f}%.".format(\
        timedelta2Minutes(activity_time), timedelta2Minutes(observation_period), 100 * timedelta2Minutes(activity_time) / timedelta2Minutes(observation_period)))

    if timedelta2Minutes(activity_time) < 1:
        print("Active interval was less then a minute, which is not enough for statistics.")
    hand_moves_per_hour = (len(mouse_to_kb) + len(kb_to_mouse)) * 60. / timedelta2Minutes(activity_time)
    hand_moving_percents = timedelta2Minutes(hand_moving_time) * 100 / timedelta2Minutes(activity_time)
    print("You have moved your hand from mouse to keyboard {} times and {} times back, you do it average {:1.1f} times per hour and this tooks you {:3.1f}% of your active time".format(
        len(mouse_to_kb), len(kb_to_mouse), (len(mouse_to_kb) + len(kb_to_mouse)) * 60 / timedelta2Minutes(activity_time), hand_moving_percents))

    kb_to_scrolling_time = calc_total_trastition_time(transition_to_scrolling)
    mean_kb_to_scrolling = calc_mean_trastition_time(transition_to_scrolling)
    print("Your mean delay to start scrolling is {}, that is total {} diring the observation."
    	.format(mean_kb_to_scrolling, kb_to_scrolling_time))

    isolated_mouse_times = [g[-1]-g[0] for g in isolated_mouse_events if g and g[-1]-g[0] < datetime.timedelta(seconds=30)]
    isolated_mouse_sum = sum(isolated_mouse_times, datetime.timedelta())
    print("You used mouse less then then 30 seconds between typing {} times during {}, mean isolated mouse usage time is {:2.1f} seconds"
        .format(len(isolated_mouse_times), isolated_mouse_sum, timedelta2Minutes(isolated_mouse_sum)*60/len(isolated_mouse_times)))
