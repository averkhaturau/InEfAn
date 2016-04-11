import sys
import datetime
import re

####################################################################
#  Global variables
parsing_header = False
is_header_info_shown = False

foreground_windows = []

activity_periods = []
                                            
# to estimate typing speed
key_press_event_groups = [[]]
shortcut_events = []  # ctrl or alt with any other key
addi_keys_events = [] # pgup, pgdown, arrow keys, home, end, ins, del, ...

# to counterpose mouse events to key presses
mouse_click_events = []
mouse_other_events = []

inactivity_interval = datetime.timedelta(seconds=4) # if no events during this time span, consider user inactive


# filtered
unique_input_events = []


analysis_begin = datetime.datetime(1917,11,7)
analysis_end = datetime.datetime.now()
analysis_current = analysis_begin # to avoid double parsing of the continued files

inefan_exit_events = []
last_parsed_time = None

keys_and_scrolls = []


class NeedParsePrevFile(Exception):
    pass


is_ctrl_key_down = False
is_alt_key_down = False

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

    global analysis_current

    if " ".join(line_word[2:5]) == "Continuing the logfile" and analysis_current < event_time:
        raise NeedParsePrevFile(" ".join(line_word[5:]).split('\\')[-1], event_time)

    last_parsed_time = event_time
    # print(event_time, line_word[2:])

    if not (analysis_begin < event_time < analysis_end):
        return

    analysis_current = event_time

    if line_word[2] == "mouse":
        on_mouse_event(event_time, line_word)
        user_is_active_at(event_time)

    elif line_word[2] == "keyboard":
        on_keyboard_event(event_time, line_word)
        user_is_active_at(event_time)

    else:
        # system event
        evt_text = " ".join(line_word[2:])
        #print((evt_text + " at " +
        #unicode(event_time)).encode(sys.stdout.encoding, errors='replace'))

        m = re.search('Foreground window title is "(.*)" from process name "(.*)" running from file "(.*)"', evt_text)
        if m:
            current_window = {
                "title": m.group(1),
                "procname":m.group(2),
                "filename":m.group(3),
            }
        else:
            current_window = {\
                "title": "",\
                "procname": "",\
                "filename": "",\
            }

        if not foreground_windows or (current_window["title"] != foreground_windows[-1][1]["title"] and current_window["filename"] != foreground_windows[-1][1]["filename"]):
            foreground_windows.append((event_time, current_window))


# determine user activity periods
def user_is_active_at(t):
    if not activity_periods or (t - activity_periods[-1][1]) > inactivity_interval:
        activity_periods.append([t,t])
    else:
        activity_periods[-1][1] = t


def on_mouse_event(event_time, line_word):
    global key_press_event_groups

    if line_word[3] in ("wheel","h-wheel"):
        keys_and_scrolls.append(("scroll " + line_word[-1], event_time))

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


def on_keyboard_event(event_time, line_word):
    global is_ctrl_key_down, is_alt_key_down, key_press_event_groups

    keys_and_scrolls.append(("key " + line_word[-1], event_time))
    if line_word[-1] == "up":
        if len(line_word) > 4:
            if line_word[4] == "CONTROL":
                is_ctrl_key_down = False
            elif line_word[4] == "MENU":
                is_alt_key_down = False

        #print("Keyboard using stopped at " + unicode(event_time))
        if (not unique_input_events or unique_input_events[-1][0] != "keyboard stopped"):
            unique_input_events.append(("keyboard stopped", event_time))
        else:
            unique_input_events[-1] = ("keyboard stopped", event_time)

        if is_ctrl_key_down or is_alt_key_down:
            shortcut_events.append(event_time)
        else: # no ctrl / alt pushed
            if line_word[3] in ("letter", "digit", "SPACEBAR", "-","+",",",".", "\\"):
                key_press_event_groups[-1].append(event_time)
            elif line_word[3] in ("PAGE", "UP", "DOWN", "LEFT", "RIGHT", "HOME", "END", "INS", "DEL", "PRINT", "PAUSE", "SCROLL", "NUM", "Add", "Subtract", "Multiply", "Divide"):
                addi_keys_events.append(event_time)

    elif line_word[-1] == "down":
        if len(line_word) > 4:
            if line_word[4] == "CONTROL":
                is_ctrl_key_down = True
            elif line_word[4] == "MENU":
                is_alt_key_down = True

        if not unique_input_events or unique_input_events[-1][0] != "keyboard started":
            unique_input_events.append(("keyboard started", event_time))
    else:
        print("Keyboard single event at " + str(event_time))
        

