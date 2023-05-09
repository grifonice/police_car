import websocket
import threading
import time
import rel
import tkinter as tk
import sys

if sys.platform == 'win32':
    key_map = {
        'w': 87,
        'a': 65,
        's': 83,
        'd': 68,
        "left": 37,
        'up': 38,
        "right": 39,
        'down': 40,
        "ctrl": 17,
        "q": 81,
        "e": 69

    }
else:
    key_map = {
        'w': 25,
        'a': 38,
        's': 39,
        'd': 40,
        'up': 111,
        'down': 116,
        "ctrl": 22,
        "q": 87,
        "e": 67
    }


stop = False
keys = []
reads = ""


def key_checker():
    keys_old = []
    current_direction = ""
    while True:
        if stop:
            return
        if keys != keys_old:
            if key_map["w"] in keys and key_map["a"] in keys:
                if not current_direction == "f":
                    ws.send("forward")
                ws.send("left")
                current_direction = "fl"
            elif key_map["w"] in keys and key_map["d"] in keys:
                if not current_direction == "f":
                    ws.send("forward")
                ws.send("right")
                current_direction = "fr"
            elif key_map["w"] in keys:
                if not current_direction == "f":
                    ws.send("forward")
                current_direction = "f"
            elif key_map["s"] in keys and key_map["a"] in keys:
                if not current_direction == "b":
                    ws.send("back")
                ws.send("left")
                current_direction = "bl"
            elif key_map["s"] in keys and key_map["d"] in keys:
                if not current_direction == "b":
                    ws.send("back")
                ws.send("right")
                current_direction = "br"
            elif key_map["s"] in keys:
                if not current_direction == "b":
                    ws.send("back")
                current_direction = "b"
            elif key_map["up"] in keys or key_map["down"] in keys or key_map["right"] in keys or key_map["left"] in keys:
                pass
            else:
                current_direction = "s"
                ws.send("stop")
            keys_old = keys[:]
        time.sleep(0.001)


def servos():
    speed = 50
    arm_angle = 0
    hand_angle = 0
    arm_max_angle = 180
    hand_max_angle = 180
    while True:
        if stop:
            return
        else:
            if key_map["left"] in keys:
                if hand_angle < hand_max_angle:
                    hand_angle += 1
                ws.send("hand "+str(hand_angle))
            elif key_map["right"] in keys:
                if hand_angle > 0:
                    hand_angle -= 1
                ws.send("hand "+str(hand_angle))
            elif key_map["up"] in keys:
                if arm_angle < arm_max_angle:
                    arm_angle += 1
                ws.send("arm "+str(arm_angle))
            elif key_map["down"] in keys:
                if arm_angle > 0:
                    arm_angle -= 1
                ws.send("arm "+str(arm_angle))

            if key_map["q"] in keys:
                if speed < 255:
                    speed += 1
                print("a")
                ws.send(f"speed {speed}")
            elif key_map["e"] in keys:
                if speed > 50:
                    speed -= 1
                print("a")
                ws.send(f"speed {speed}")
        time.sleep(0.001)
temp=""
def on_message(wsapp, message):
    global temp
    temp=message


def ping_status():
    old_time = 0
    global temp
    while True:
        old_time = time.time()
        ws.send("ping")
        while temp != "pong" and old_time+1000 > time.time():
            time.sleep(0.001)
            if stop:
                exit()
        temp = ""
        print(time.time-old_time)



ws = websocket.WebSocketApp(
    "ws://192.168.3.43:80/ws", on_message=on_message)


def wscoso():

    ws.run_forever()


thread = threading.Thread(target=key_checker)
thread2 = threading.Thread(target=servos)
thread3 = threading.Thread(target=ping_status)
thread4 = threading.Thread(target=wscoso)

win = tk.Tk()


def key_pressed(key):
    if not key.keycode in keys:
        keys.append(key.keycode)
        if key.keycode == key_map["ctrl"]:
            ws.send("speed 255")


def key_released(key):
    keys.remove(key.keycode)
    if key.keycode == key_map["ctrl"]:
        ws.send("speed 50")


if __name__ == "__main__":
    websocket.enableTrace(True)
    win.bind('<KeyPress>', key_pressed)
    win.bind('<KeyRelease>', key_released)
    thread4.start()
    thread.start()
    thread2.start()
    thread3.start()
    win.mainloop()
    stop = True
