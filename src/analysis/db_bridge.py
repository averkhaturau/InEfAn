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
        query = ("insert into apps (app_name, app_path, app_domain) values ('{}', '{}', '{}') ON DUPLICATE KEY UPDATE app_id=LAST_INSERT_ID(app_id) ".format(app_name, app_path, app_domain,app_name))
        cursor.execute(query)
        return cursor.lastrowid
