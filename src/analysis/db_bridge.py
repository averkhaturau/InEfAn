import datetime
import stat_settings
import utils

no_db = False

try:
    import db_secret
    import MySQLdb
except ImportError:
    no_db = True
    pass


def register_app(app_name, app_path, app_domain):
    if no_db:
        return -1
    with MySQLdb.connect(*db_secret.db_connect_params) as cursor:
        query = (
        '''insert into apps (app_name, app_path, app_domain)
        values ('{}', '{}', '{}')
        ON DUPLICATE KEY UPDATE app_id=LAST_INSERT_ID(app_id)'''
        .format(app_name, app_path, app_domain))
        cursor.execute(query)
        return cursor.lastrowid


def register_stat_type(stat_name):
    if no_db:
        return -1
    with MySQLdb.connect(*db_secret.db_connect_params) as cursor:
        query = (
        '''insert into stat_types (stat_type_name)
        values ('{}')
        ON DUPLICATE KEY UPDATE stat_type_id=LAST_INSERT_ID(stat_type_id)'''
        .format(stat_name))
        cursor.execute(query)
        return cursor.lastrowid


def add_stat(stat_type_id, app_id, machine_id, start_time, value):
    if no_db:
        return -1

    if type(value) is list:
        val = value
    else:
        val = [value]

    if type(val[0]) is int:
        val_column = "value_int"
    else:
        val_column = "value_ts"

    time = start_time
    interval = stat_settings.stat_interval

    with MySQLdb.connect(*db_secret.db_connect_params) as cursor:
        for v in val:
            query = (
            ''' insert into stats (stat_type_id, app_id, machine_id, stat_time, {})
            values({}, {}, "{}", "{}", "{}")
            '''.format(val_column, stat_type_id, app_id, machine_id, time, v)
            )
            time += interval
            if v:
                cursor.execute(query)


def save_events_to_db(stat_name, app_id, machine_id, events_list):
    if events_list:
        stat_type_id = register_stat_type(stat_name)
        start_time = stat_settings.to_start(events_list[0])
        events_hist = utils.norm_events_stat_to_hist(
            events_list,
            start_time,
            stat_settings.to_finish(events_list[-1]),
            stat_settings.stat_interval)
        add_stat(stat_type_id, app_id, machine_id, start_time, events_hist)


