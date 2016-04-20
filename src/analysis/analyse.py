import sys
import datetime

# local
import log_parse
import chart
import characteristics
import utils
import os
import glob

import db_bridge
import stat_settings

# for pyinstaller
try:
    # for Python2
    import Tkinter
    import FileDialog
except ImportError:
    # for Python3
    import tkinter
    import tkinter.filedialog 


def parse_log():

    if sys.version_info < (3, 0):
        reload(sys)  
        sys.setdefaultencoding('utf8')
        fopen_func = lambda filename: open(filename)
    else:
        fopen_func = lambda filename: open(filename, encoding='utf8')

    if len(sys.argv) < 2 or not os.path.exists(sys.argv[1]):
        print("Usage:\n\t>" + sys.argv[0] + " inefan.log [-begin <analysis start time> -end <analysis end time>]")
        exit(-1)

    if os.path.isdir(sys.argv[1]):
        # find the last file
        logfile = max(glob.iglob(sys.argv[1] + '/log*.txt'), key=os.path.basename)
    else:
        logfile = sys.argv[1]

    if len(sys.argv) > 3:
        if sys.argv[2] == "-begin":
            log_parse.analysis_begin = datetime.datetime.strptime(sys.argv[3], "%Y-%m-%d %H:%M:%S")
            if len(sys.argv) > 5 and sys.argv[4] == "-end":
                log_parse.analysis_end = datetime.datetime.strptime(sys.argv[5], "%Y-%m-%d %H:%M:%S")
        elif sys.argv[2] == "-last":
            passed_time_interval = datetime.datetime.strptime(sys.argv[3], "%H:%M")
            log_parse.analysis_begin = datetime.datetime.now() - datetime.timedelta(hours=passed_time_interval.hour, minutes=passed_time_interval.minute)

        print("Analysing logs from {} to {}".format(str(log_parse.analysis_begin), str(log_parse.analysis_end)))

    path_steps = os.path.split(logfile)
    log_dir = path_steps[0]
    files_to_parse = [path_steps[1]]

    global machine_id
    machine_id = os.path.split(path_steps[0])[1]
    print("machine_id is "+ machine_id)


    last_log_time = log_parse.analysis_current
    log_start_time = {}
    while files_to_parse:
        try:
            with fopen_func(log_dir + '/' + files_to_parse[0]) as logfile:
                #print("parsing " + files_to_parse[0])
                for line in logfile:
                        log_parse.handle_log_line(line)
            if files_to_parse[0] in log_start_time:
                log_parse.analysis_current = log_start_time[files_to_parse[0]]
            files_to_parse = files_to_parse[1:]
        except log_parse.NeedParsePrevFile as nppf:
            log_start_time.update({nppf.args[0]: nppf.args[1]})
            files_to_parse = [nppf.args[0]] + files_to_parse
            last_log_time = nppf.args[1]
            # print("Parsing {}".format(files_to_parse))
        except (OSError, IOError) as ee:
            print(ee)
            log_parse.analysis_current = last_log_time
            files_to_parse = files_to_parse[1:]
            print("Parsing {}".format(files_to_parse))


parse_log()


apps_usage = characteristics.appsStatistics(log_parse.foreground_windows)
print("Most user apps are: {}".format(apps_usage[:6]))


app_intrvls = utils.app_intervals(log_parse.foreground_windows)
for procname in app_intrvls:
    if not procname:
        continue
    print("\n\nFor the '{}' we gathed the following statistics:".format(procname))
    app_id = db_bridge.register_app(procname, None, None)
    print("app {} has id {}".format(procname, app_id))
    app_activity_periods   = utils.cross_intervals(app_intrvls[procname], log_parse.activity_periods, lambda element: element, lambda orig,intrvl: intrvl)
    app_input_events       = utils.cross_intervals(app_intrvls[procname], log_parse.unique_input_events, lambda element: [element[1],element[1]], lambda orig,intrvl: (orig[0],intrvl[1]))
    app_keys_and_scrolls   = utils.cross_intervals(app_intrvls[procname], log_parse.keys_and_scrolls, lambda element: [element[1],element[1]], lambda orig,intrvl: (orig[0],intrvl[1]))
    app_mouse_click_events = utils.cross_intervals(app_intrvls[procname], log_parse.mouse_click_events, lambda element: [element,element], lambda orig,intrvl: intrvl[1])
    app_mouse_other_events = utils.cross_intervals(app_intrvls[procname], log_parse.mouse_other_events, lambda element: [element,element], lambda orig,intrvl: intrvl[1])
    app_shortcut_events    = utils.cross_intervals(app_intrvls[procname], log_parse.shortcut_events, lambda element: [element,element], lambda orig,intrvl: intrvl[1])
    app_addi_keys_events   = utils.cross_intervals(app_intrvls[procname], log_parse.addi_keys_events, lambda element: [element,element], lambda orig,intrvl: intrvl[1])
    app_key_press_event_groups = []
    for key_evt_group in log_parse.key_press_event_groups:
        app_event_group = utils.cross_intervals(app_intrvls[procname], key_evt_group, lambda element: [element,element], lambda orig,intrvl: intrvl[1])
        if app_event_group:
            app_key_press_event_groups.append(app_event_group)
    app_stat = characteristics.print_characteristics(app_activity_periods, log_parse.inactivity_interval, app_key_press_event_groups, app_input_events, app_keys_and_scrolls, app_shortcut_events, app_addi_keys_events)
    #print("{} statistics is {}".format(procname, app_stat))
    #if not app_stat or not app_stat["mouse_to_kb"] or not app_stat["key_press_events"]:
    #    continue
    #local_foreground = [(app_stat["mouse_to_kb"][0][0], {"title": procname, "procname": procname, "filename": ""})]
    #chart.plot_transitions(app_stat["kb_to_mouse"], app_stat["mouse_to_kb"], local_foreground, "res/" + procname + "-plot.png")
    #chart.log_plot(app_stat["key_press_events"], app_mouse_click_events, app_mouse_other_events, local_foreground, log_parse.inefan_exit_events, "res/" + procname + "-graph.png")

    # write statistics to database
    db_bridge.save_events_to_db("mouse_clicks", app_id, machine_id, app_mouse_click_events)
    db_bridge.save_events_to_db("shortcuts", app_id, machine_id, app_shortcut_events)
    if app_stat and "key_press_events" in app_stat and app_stat["key_press_events"]:
        db_bridge.save_events_to_db("keypresses", app_id, machine_id, app_stat["key_press_events"])
    db_bridge.save_events_to_db("addikeypresses", app_id, machine_id, app_addi_keys_events)


print("\n\nGeneral statistics:")
main_stat = characteristics.print_characteristics(log_parse.activity_periods, log_parse.inactivity_interval, log_parse.key_press_event_groups, log_parse.unique_input_events, log_parse.keys_and_scrolls, log_parse.shortcut_events, log_parse.addi_keys_events)

key_press_events = main_stat["key_press_events"]
mouse_to_kb      = main_stat["mouse_to_kb"]
kb_to_mouse      = main_stat["kb_to_mouse"]


#chart.plot_transitions(kb_to_mouse, mouse_to_kb, log_parse.foreground_windows, "res/all-plot.png")
#chart.log_plot(key_press_events, log_parse.mouse_click_events, log_parse.mouse_other_events, log_parse.foreground_windows, log_parse.inefan_exit_events, "res/all-chart.png")

# write statistics to database
app_id = 0
db_bridge.save_events_to_db("mouse_clicks", app_id, machine_id, app_mouse_click_events)
db_bridge.save_events_to_db("shortcuts", app_id, machine_id, app_shortcut_events)
if app_stat and "key_press_events" in app_stat and app_stat["key_press_events"]:
    db_bridge.save_events_to_db("keypresses", app_id, machine_id, app_stat["key_press_events"])
db_bridge.save_events_to_db("addikeypresses", app_id, machine_id, app_addi_keys_events)

