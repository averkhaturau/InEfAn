import sys
import datetime
import log_parse
import log_plot


def parse_log():

    if sys.version_info < (3, 0):
        reload(sys)  
        sys.setdefaultencoding('utf8')
        fopen_func = lambda filename: open(filename)
    else:
        fopen_func = lambda filename: open(filename, encoding='utf8')

    if len(sys.argv) < 2:
        print("Usage:\n\t>" + sys.argv[0] + " inefan.log [-begin <analysis start time> -end <analysis end time>]")
        exit(-1)

    if len(sys.argv) > 3:
        if sys.argv[2] == "-begin":
            log_parse.analysis_begin = datetime.datetime.strptime(sys.argv[3], "%Y-%m-%d %H:%M:%S")
            if len(sys.argv) > 5 and sys.argv[4] == "-end":
                log_parse.analysis_end = datetime.datetime.strptime(sys.argv[5], "%Y-%m-%d %H:%M:%S")
        elif sys.argv[2] == "-last":
            passed_time_interval = datetime.datetime.strptime(sys.argv[3], "%H:%M")
            log_parse.analysis_begin = datetime.datetime.now() - datetime.timedelta(hours=passed_time_interval.hour, minutes=passed_time_interval.minute)

        print("Analysing logs from {} to {}".format(str(log_parse.analysis_begin), str(log_parse.analysis_end)))                


    with fopen_func(sys.argv[1]) as logfile:
        for line in logfile:
            try:
                log_parse.handle_log_line(line)
            except IOError as ee:
                print("I/O error({0}): {1}".format(ee.errno, ee.strerror))
            except ValueError as ee:
                print("ValueError: {}\non parsing '{}'".format(str(ee), line))
            except:
                print("Unexpected error {} on parsing line '{}'".format(sys.exc_info()[0], line))

parse_log()

log_parse.print_characteristics()
log_plot.log_plot(log_parse.key_press_events, log_parse.mouse_click_events, log_parse.mouse_other_events, log_parse.foreground_windows)
