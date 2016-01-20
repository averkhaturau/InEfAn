import sys
import datetime
import re


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
            print("Mouse using stopped at " + unicode(event_time))
        elif line_word[-1] == "down" or line_word[-1] == "started":
            print("Mouse using started at " + unicode(event_time))
        else:
            print("Mouse single event at " + unicode(event_time))

        user_is_active_at(event_time)

    elif line_word[2] == "keyboard":
        if line_word[-1] == "up":
            print("Keyboard using stopped at " + unicode(event_time))
            if line_word[3] in ("letter", "digit", "SPACEBAR", "-","+",",",".", "\\"):
                key_press_events.append(event_time)
        elif line_word[-1] == "down":
            print("Keyboard using started at " + unicode(event_time))
        else:
            print("Keyboard single event at " + unicode(event_time))

        user_is_active_at(event_time)

    else:
        # system event
        print(line_word)
        evt_text = " ".join(line_word[2:])
        print((evt_text + " at " + unicode(event_time)).encode(sys.stdout.encoding, errors='replace'))

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
    it = iter(activity_periods)
    for period_start in it:
        period_end = next(it)
        if period_end - period_start < inactivity_interval:
            continue
        num_keypresses = sum(1 for press_time in key_press_events if press_time >= period_start and period_start <= period_end)
        if num_keypresses < 2:
            continue
        typing_speed = num_keypresses*60 / (period_end - period_start).seconds
        print("Typing speed is {}".format(typing_speed))
        print(period_start, period_end, num_keypresses, typing_speed)






parse_log()

print_characteristics()

