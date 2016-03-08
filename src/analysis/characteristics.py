import math
import datetime

from utils import *


def print_characteristics(activity_periods, inactivity_interval, key_press_event_groups, unique_input_events, keys_and_scrolls):

    # output values
    key_press_events = []
    mouse_to_kb = []
    kb_to_mouse = []


    activity_time = datetime.timedelta()

    transition_to_scrolling = []
    isolated_mouse_events = [[]]
    typing_keypresses_intervals = []

    for period in activity_periods:
        period_len = period[1] - period[0]
        if period_len < inactivity_interval:
            continue

        activity_time += period_len
        
        # calculate typing speed
        key_presses_in_period = []
        for group in key_press_event_groups:
            filtered_group = list(filter(lambda press_time: period[0] <= press_time <= period[1], group))
            if len(filtered_group) > 3:
                key_presses_in_period.append(filtered_group)

        if key_presses_in_period:
            key_press_events += flattern(key_presses_in_period)

        typing_keypresses_intervals += list(map(lambda g: (len(g), g[-1] - g[0]), key_presses_in_period))

        events_scope = list(filter(lambda eType_eTime: period[0] <= eType_eTime[1] <= period[1], unique_input_events))

        # calcuate mouse-to-keyboard switch time
        for (e1,e2) in pairwise(events_scope):
            if e1[0] == "mouse stopped" and e2[0] == "keyboard started":
                mouse_to_kb.append((e1[1],e2[1]))
            elif e1[0] == "keyboard stopped" and e2[0] == "mouse started":
                kb_to_mouse.append((e1[1],e2[1]))

        # calc isolated mouse usages, e.g.  time on mouse in activity periods,
        # between key presses
        gathering = False
        for evt in events_scope:
            if evt[0] == "keyboard stopped" and not gathering:
                gathering = True
                if not isolated_mouse_events or isolated_mouse_events[-1]:
                    isolated_mouse_events.append([])
            elif evt[0] == "keyboard started":
                gathering = False
            else:
                if gathering:
                     isolated_mouse_events[-1].append(evt[1])
        if gathering and isolated_mouse_events[-1]:
            isolated_mouse_events.pop()

        # calc hand-transition time for scrolling only
        transition_to_scrolling_in_period = list(filter(lambda e1_e2: e1_e2[0][0] == "key up" and e1_e2[1][0] == "scroll started",
            pairwise(list(filter(lambda evt: period[0] <= evt[1] <= period[1], keys_and_scrolls)))))
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

    # save all hand transitions to a file
    #with open('res/all-transitions.txt', 'w') as transitionsFile:
    #    transitionsFile.write('kb_to_mouse='+str(kb_to_mouse)+'\n')
    #    transitionsFile.write('mouse_to_kb='+str(mouse_to_kb)+'\n\n')


    typing_speed_variance = math.sqrt(sum(map(lambda s_i: (calc_typing_speed(s_i[0], s_i[1]) - mean_typing_speed) ** 2, typing_keypresses_intervals)) / len(typing_keypresses_intervals))

    print("Mean Typing speed is {:1.1f}, variance is {}".format(mean_typing_speed, typing_speed_variance))

    mean_mouse_to_kb = calc_mean_trastition_time(mouse_to_kb)
    mouse_to_kb_variance = math.sqrt(sum(map(lambda s_f: (timedelta2Minutes(s_f[1] - s_f[0] - mean_mouse_to_kb) * 60) ** 2, mouse_to_kb)) / len(mouse_to_kb))
    print("Mean time to transit hand from mouse to keyboard = {}, variance = {} seconds.".format(mean_mouse_to_kb, mouse_to_kb_variance))
    mean_kb_to_mouse = calc_mean_trastition_time(kb_to_mouse)
    kb_to_mouse_variance = math.sqrt(sum(map(lambda s_f: (timedelta2Minutes(s_f[1] - s_f[0] - mean_kb_to_mouse) * 60) ** 2, kb_to_mouse)) / len(kb_to_mouse))
    print("Mean time to transit hand from keyboard to mouse = {}, variance = {} seconds.".format(mean_kb_to_mouse, kb_to_mouse_variance))
    hand_moving_time = calc_total_trastition_time(mouse_to_kb + kb_to_mouse)
    print("During the observation you moved your hand total {}.".format(hand_moving_time))

    observation_period = unique_input_events[-1][1] - unique_input_events[0][1]
    # extrapolate statistics to 1 year
    one_year_rate = 365.25 * 24 * 60 / timedelta2Minutes(observation_period)
    print("For 1 year you would spend {:1.0f} hours of you life to move you hand to mouse and back, if you use you PC like you do during the observed time."
        .format(timedelta2Minutes(hand_moving_time) * one_year_rate / 60))


    if timedelta2Minutes(observation_period) < 1:
        print("Observation time is not enough for statistics...")

    print("You were active {:1.1f} minutes during {:1.1f} observed, which is {:1.1f}%.".format(\
        timedelta2Minutes(activity_time), timedelta2Minutes(observation_period), 100 * timedelta2Minutes(activity_time) / timedelta2Minutes(observation_period)))

    if timedelta2Minutes(activity_time) < 1:
        print("Active interval was less then a minute, which is not enough for statistics.")
    hand_moves_per_hour = (len(mouse_to_kb) + len(kb_to_mouse)) * 60. / timedelta2Minutes(activity_time)
    hand_moving_percents = timedelta2Minutes(hand_moving_time) * 100 / timedelta2Minutes(activity_time)
    print("You have moved your hand from mouse to keyboard {} times and {} times back, you do it average {:1.1f} times per hour and this tooks you {:3.1f}% of your active time"
        .format(len(mouse_to_kb), len(kb_to_mouse), (len(mouse_to_kb) + len(kb_to_mouse)) * 60 / timedelta2Minutes(activity_time), hand_moving_percents))

    kb_to_scrolling_time = calc_total_trastition_time(transition_to_scrolling)
    mean_kb_to_scrolling = calc_mean_trastition_time(transition_to_scrolling)
    print("Your mean delay to start scrolling is {}, that is total {} diring the observation."
        .format(mean_kb_to_scrolling, kb_to_scrolling_time))

    isolated_mouse_times = [g[-1] - g[0] for g in isolated_mouse_events if g and g[-1] - g[0] < datetime.timedelta(seconds=30)]
    isolated_mouse_sum = sum(isolated_mouse_times, datetime.timedelta())
    if isolated_mouse_times:
        print("You used mouse less then then 30 seconds between typing {} times during {}, mean isolated mouse usage time is {:2.1f} seconds"
            .format(len(isolated_mouse_times), isolated_mouse_sum, timedelta2Minutes(isolated_mouse_sum) * 60 / len(isolated_mouse_times)))


    return {
        "key_press_events":key_press_events,
        "mouse_to_kb": mouse_to_kb,
        "kb_to_mouse": kb_to_mouse}
