import datetime

# aggregate statistics by this interval
stat_interval = datetime.timedelta(hours=1)


def to_start(dt):
    if type(dt) is datetime.datetime:
        return dt.replace(minute=0, second=0, microsecond=0)
    else:
        return dt[0].replace(minute=0, second=0, microsecond=0)


def to_finish(dt):
    if type(dt) is datetime.datetime:
        return (dt + stat_interval).replace(minute=0, second=0, microsecond=0)
    else:
        return (dt[0] + stat_interval).replace(minute=0, second=0, microsecond=0)

