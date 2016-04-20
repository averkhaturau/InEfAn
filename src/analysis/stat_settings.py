import datetime

# aggregate statistics by this interval
stat_interval = datetime.timedelta(hours=1)


def to_start(dt):
    return dt.replace(minute=0, second=0, microsecond=0)


def to_finish(dt):
    return (dt + stat_interval).replace(minute=0, second=0, microsecond=0)

