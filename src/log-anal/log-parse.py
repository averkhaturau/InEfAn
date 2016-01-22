import sys
import datetime
import re
from itertools import *


def pairwise(iterable):                    
    "s -> (s0,s1), (s1,s2), (s2, s3), ..." 
    a, b = tee(iterable)                   
    try:                                   
        next(b)                            
    except StopIteration:                  
        pass                               
    return zip(a, b)                    
   

def parse_log():
    reload(sys)  
    sys.setdefaultencoding('utf-8')
 #   sys.stdout = open(sys.stdout.fileno(), mode='w', encoding='utf8', buffering=1)

    if len(sys.argv) < 2:
        print("Usage:\n\t>" + sys.argv[0] + " inefan.log")
        exit(-1)

    with open(sys.argv[1]) as logfile:
        for line in logfile:
           handle_log_line(line[:-1])



####################################################################
#  Global variables
parsing_header = False

is_using_mouse = False
is_using_keyboard = False

foreground_windows = {}

activity_periods = []
key_press_events = []

inactivity_interval = datetime.timedelta(seconds=4) # if no events during this time span, consider user inactive

mean_typing_speed = 0.0

# filtered 
unique_input_events = []

mouse_to_kb = []
kb_to_mouse = []

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
    m = re.search("(?<=running on)[ \\t]+(.*)[ \\t]+powered by[ \\t]+(.*)", line)
    if m:
    	global pc_type,os_version
        pc_type = m.group(1)
        os_version = m.group(2)
        print("Running on " + pc_type + " powered by " + os_version)


# read simple event line
def parse_log_line(line):

    line_word = filter(None, re.split(" |\t|\n|\r", line))
    if len(line_word) < 3:
        return
    event_time = datetime.datetime.strptime( line_word[0] + " " + line_word[1], "%Y-%m-%d %H:%M:%S.%f" )
    # print(event_time, line_word[2:])

    global current_window

    if line_word[2] == "mouse":
        if line_word[-1] == "up" or line_word[-1] == "finished":
            #print("Mouse using stopped at " + unicode(event_time))
            if (unique_input_events or unique_input_events[-1][0] != "mouse stopped"):
                unique_input_events.append(("mouse stopped", event_time))
            else:
                unique_input_events[-1] = ("mouse stopped", event_time)
        elif line_word[-1] == "down" or line_word[-1] == "started":
            #print("Mouse using started at " + unicode(event_time))
            if (not unique_input_events or unique_input_events[-1][0] != "mouse started"):
                unique_input_events.append(("mouse started", event_time))
        else:
            print("Mouse single event at " + unicode(event_time))

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
            print("Keyboard single event at " + unicode(event_time))

        user_is_active_at(event_time)

    else:
        # system event
        evt_text = " ".join(line_word[2:])
        #print((evt_text + " at " + unicode(event_time)).encode(sys.stdout.encoding, errors='replace'))

        m = re.search('Foreground window title is "(.*)" from process name "(.*)" running from file "(.*)"', evt_text)
        if m:
            current_window = {\
                "title": m.group(1),\
                "procName":m.group(2),\
                "filename":m.group(3)\
            }
        else:
            current_window = None # FIXME
        foreground_windows.update({event_time: current_window})


# determine user activity periods
def user_is_active_at(t):
    if len(activity_periods) == 0 or (t - activity_periods[-1]) > inactivity_interval:
        activity_periods.append(t)
        activity_periods.append(t)
    else:
        activity_periods[-1] = t


def print_characteristics():
    global mean_typing_speed, mouse_to_kb, kb_to_mouse
    it = iter(activity_periods)
    for period_start in it:
        period_end = next(it)
        if period_end - period_start < inactivity_interval:
            continue
        # calculate typing speed
        num_keypresses = sum(1 for press_time in key_press_events if period_start <= press_time <= period_end)
        if num_keypresses < 2:
            continue
        typing_speed = num_keypresses*60 / (period_end - period_start).seconds
        mean_typing_speed += typing_speed * 2 / len(activity_periods)
        #print("Typing speed is {} at {}".format(typing_speed, period_end))
        # print(period_start, period_end, num_keypresses, typing_speed)

        # calcuate mouse-to-keyboard switch time
        events_scope = filter(lambda (eType, eTime): period_start <= eTime <= period_end, unique_input_events)
        for (e1,e2) in pairwise(events_scope):
            if e1[0] == "mouse stopped" and e2[0] == "keyboard started":
                mouse_to_kb.append((e1[1],e2[1]))
            elif e1[0] == "keyboard stopped" and e2[0] == "mouse started":
                kb_to_mouse.append((e1[1],e2[1]))
        

    print("Mean Typing speed is {}".format(mean_typing_speed))

    mean_mouse_to_kb = reduce(lambda x,y: x+y, map(lambda (e1,e2): e2-e1, mouse_to_kb))/len(mouse_to_kb)
    print("Mean time to transit hand from mouse to keyboard = {}".format(mean_mouse_to_kb))
    mean_kb_to_mouse = reduce(lambda x,y: x+y, map(lambda (e1,e2): e2-e1, kb_to_mouse))/len(kb_to_mouse)
    print("Mean time to transit hand from keyboard to mouse = {}".format(mean_kb_to_mouse))



parse_log()

print_characteristics()