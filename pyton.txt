import serial
import cv2
import threading
import time
import mediapipe as mp
import math

arduino = serial.Serial(port='COM4', baudrate=115200, timeout=1)

servo_max = 255
fingers = [0,0,0,0,0]

def send_data_to_arduino():
    while True:
        data = ','.join([str(val) for val in fingers]) + '\n'
        arduino.write(data.encode())
        arduino.flush()
        response = arduino.readline().decode('utf-8').rstrip()
        print(response)
        time.sleep(0.1)  # Wysyłaj dane co sekundę


def map_value(x, in_min, in_max, out_min, out_max):
    val = int((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min)
    if val < 1:
        val =7
    if val > servo_max:
        val = servo_max
    return val


def isClosed(Tx,Ty,cx,cy):
    distance = math.sqrt((cx - Tx)**2 + (cy - Ty)**2)
    return int(distance)


def process_camera():
    global fingers
    modifier_x, modifier_y = -10, -30
    cap = cv2.VideoCapture(0)  # Otwórz kamerę
    cap.set(3, 1280)
    mphands = mp.solutions.hands
    Hands = mphands.Hands(max_num_hands=1, min_detection_confidence=0.7, min_tracking_confidence=0.6)
    mpdraw = mp.solutions.drawing_utils
    while True:
        ret, frame = cap.read()
        if not ret:
            break
        RGBframe = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        result = Hands.process(RGBframe)
        if result.multi_hand_landmarks:

            for handLm in result.multi_hand_landmarks:

                for id, lm in enumerate(handLm.landmark):
                    h, w, _ = frame.shape
                    cx, cy = int(lm.x * w), int(lm.y * h)

                    if id == 0:
                        Tx, Ty = cx, cy
                    if id == 4:
                        cv2.putText(frame, '1', (cx + modifier_x, cy + modifier_y), cv2.FONT_HERSHEY_PLAIN, 2,
                                    (255, 100, 100), 6, cv2.LINE_AA)
                        Kx, Ky = cx, cy
                    if id == 5:
                        Gx, Gy = cx, cy
                    if id == 8:
                        cv2.putText(frame, '2', (cx + modifier_x, cy + modifier_y), cv2.FONT_HERSHEY_PLAIN, 2,
                                    (255, 100, 100), 6, cv2.LINE_AA)
                        index = isClosed(Tx, Ty, cx, cy)
                    if id == 12:
                        cv2.putText(frame, '3', (cx + modifier_x, cy + modifier_y), cv2.FONT_HERSHEY_PLAIN, 2,
                                    (255, 100, 100), 6, cv2.LINE_AA)
                        middle = isClosed(Tx, Ty, cx, cy)
                    if id == 16:
                        cv2.putText(frame, '4', (cx + modifier_x, cy + modifier_y), cv2.FONT_HERSHEY_PLAIN, 2,
                                    (255, 100, 100), 6, cv2.LINE_AA)
                        ring = isClosed(Tx, Ty, cx, cy)
                    if id == 17:
                        thumb = isClosed(Kx, Ky, cx, cy)
                        good = isClosed(Gx, Gy, cx, cy)
                        if good > 120 and good < 155:
                            hand_in_range = True
                            cv2.rectangle(frame, (1000, 25), (1100, 125), (0, 255, 0), cv2.FILLED)
                            cv2.putText(frame, str(good), (1000, 150), cv2.FONT_HERSHEY_PLAIN,
                                        2, (255, 255, 255), 6, cv2.LINE_AA)
                        else:
                            hand_in_range = False
                            cv2.rectangle(frame, (1000, 25), (1100, 125), (0, 0, 255), cv2.FILLED)
                            cv2.putText(frame, str(good), (1000, 150), cv2.FONT_HERSHEY_PLAIN,
                                        2, (255, 255, 255), 6, cv2.LINE_AA)
                    if id == 20:
                        cv2.putText(frame, '5', (cx + modifier_x, cy + modifier_y), cv2.FONT_HERSHEY_PLAIN, 2,
                                    (255, 100, 100), 6, cv2.LINE_AA)
                        pinky = isClosed(Tx, Ty, cx, cy)
            cv2.rectangle(frame, (180, 5), (650, 180), (0, 0, 0), cv2.FILLED)
            cv2.line(frame, (180, 5), (650, 5), (255, 255, 0), 5)
            cv2.line(frame, (180, 180), (650, 180), (0, 255, 255), 2)
            if hand_in_range:
                thumb = map_value(thumb, 320, 40, 0, servo_max)
                index = map_value(index, 400, 110, 0, servo_max)
                middle = map_value(middle, 400, 100, 0, servo_max)
                ring = map_value(ring, 400, 100, 0, servo_max)
                pinky = map_value(pinky, 340, 80, 0, servo_max)
                fingers = [thumb, index, middle, ring, pinky]
            else:
                thumb, index, middle, ring, pinky = 0, 0, 0, 0, 0
                fingers = [thumb, index, middle, ring, pinky]

            cv2.rectangle(frame, (190, 5), (240, thumb), (255, 255, 255), cv2.FILLED)
            cv2.rectangle(frame, (290, 5), (340, index), (255, 255, 255), cv2.FILLED)
            cv2.rectangle(frame, (390, 5), (440, middle), (255, 255, 255), cv2.FILLED)
            cv2.rectangle(frame, (490, 5), (540, ring), (255, 255, 255), cv2.FILLED)
            cv2.rectangle(frame, (590, 5), (640, pinky), (255, 255, 255), cv2.FILLED)

            cv2.putText(frame, str(thumb), (190, 220), cv2.FONT_HERSHEY_PLAIN,
                        2, (255, 255, 255), 6, cv2.LINE_AA)
            cv2.putText(frame, str(index), (290, 220), cv2.FONT_HERSHEY_PLAIN,
                        2, (255, 255, 255), 6, cv2.LINE_AA)
            cv2.putText(frame, str(middle), (390, 220), cv2.FONT_HERSHEY_PLAIN,
                        2, (255, 255, 255), 6, cv2.LINE_AA)
            cv2.putText(frame, str(ring), (490, 220), cv2.FONT_HERSHEY_PLAIN,
                        2, (255, 255, 255), 6, cv2.LINE_AA)
            cv2.putText(frame, str(pinky), (590, 220), cv2.FONT_HERSHEY_PLAIN,
                        2, (255, 255, 255), 6, cv2.LINE_AA)

        cv2.imshow('autor: Mateusz Kamyszek', frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break
    cap.release()
    cv2.destroyAllWindows()

arduino_thread = threading.Thread(target=send_data_to_arduino)
arduino_thread.daemon = True
arduino_thread.start()
process_camera()
