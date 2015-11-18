#coding=utf-8
import sys
import pymysql as mdb

uhost='localhost'
uport = 3307
uuser='root'
upasswd='usbw'
udb ='e'

def mdb_cnn():
    try:
        con = mdb.connect(host=uhost,port=uport,user=uuser,passwd=upasswd,db=udb,charset='utf8')
        cur = con.cursor()
        cur.execute("SELECT VERSION()")
        data = cur.fetchone()
        print ("Mysql version : %s " % data)
        cur.execute("set time_zone = '+8:00'")
        cur.execute("set global event_scheduler = on")
        cur.execute("set global max_connect_errors=1000")
        cur.execute("set global max_connections=5000")
        cur.execute("set global wait_timeout=100")
        cur.execute("set global interactive_timeout=100")
        cur.execute("set global key_buffer_size= 256000000")
        cur.execute("set global max_allowed_packet= 1048576")     #16M=16777216;1M=1048576
    except mdb.Error as e:
        print ("Mysql Error %d: %s" % (e.args[0], e.args[1]))
        sys.exit()
    finally:
         if con:
            cur.close()
            con.close()

def mdb_call(sql):
    try:
        con = mdb.connect(host=uhost,port=uport,user=uuser,passwd=upasswd,db=udb,charset='utf8')
        cur = con.cursor(mdb.cursors.DictCursor)
        cur.execute(sql)
        con.commit()
        rows = int(cur.rowcount)
        rs = cur.fetchall()
        return rs,rows
    except mdb.Error as e:
        print ("Mysql Error %d: %s" % (e.args[0], e.args[1]))
    finally:
         if con:
            cur.close()
            con.close()

def mdb_insert(sql):
    try:
        con = mdb.connect(host=uhost,port=uport,user=uuser,passwd=upasswd,db=udb,charset='utf8')
        cur = con.cursor(mdb.cursors.DictCursor)
        cur.execute(sql)
        con.commit()
    except mdb.Error as e:
        print ("Mysql Error %d: %s" % (e.args[0], e.args[1]))
    finally:
         if con:
            cur.close()
            con.close()

def mdb_select(sql):
    try:
        con = mdb.connect(host=uhost,port=uport,user=uuser,passwd=upasswd,db=udb,charset='utf8')
        cur = con.cursor(mdb.cursors.DictCursor)
        cur.execute(sql)
        rows = int(cur.rowcount)
        rs = cur.fetchall()
        return rs,rows
    except mdb.Error as e:
        print ("Mysql Error %d: %s" % (e.args[0], e.args[1]))
    finally:
         if con:
            cur.close()
            con.close()