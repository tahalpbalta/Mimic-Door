import face_recognition
import cv2
import numpy as np
from deepface import DeepFace
import pandas as pd
import serial
from datetime import datetime
import os

df = pd.read_csv("kayit.csv", delimiter=';')

ser = serial.Serial('COM5', 9600)

folder_path = 'faces'
known_face_encodings = []
known_face_names = []
if os.path.exists(folder_path):
    files = os.listdir(folder_path)
    for file_name in files:
            image = face_recognition.load_image_file(os.path.join(folder_path, file_name))
            known_face_encodings.append(face_recognition.face_encodings(image)[0])
            file_name = file_name.split('.')[0]
            name,surname = file_name.split('_')
            known_face_names.append(f"{name} {surname}")


video_capture = cv2.VideoCapture(0)

# Duygu tanıma için gerekli ayarlamalar ve kodlar
model = DeepFace.build_model("Emotion")
emotion_labels = ['sinirli', 'igrenme', 'korku', 'mutlu', 'uzgun', 'surprise', 'dogal']
face_cascade = cv2.CascadeClassifier(cv2.data.haarcascades + 'haarcascade_frontalface_default.xml')

while True:
    _ , frame = video_capture.read()
    small_frame = cv2.resize(frame, (0, 0), fx=0.25, fy=0.25)
    rgb_small_frame = np.ascontiguousarray(small_frame[:, :, ::-1])

    # Yüz tanıma
    face_locations = face_recognition.face_locations(rgb_small_frame)
    face_encodings = face_recognition.face_encodings(rgb_small_frame, face_locations)

    for face_encoding in face_encodings:
        matches = face_recognition.compare_faces(known_face_encodings, face_encoding)
        face_distances = face_recognition.face_distance(known_face_encodings, face_encoding)
        best_match_index = np.argmin(face_distances)
        # Taninirsa
        if matches[best_match_index]:
            name = known_face_names[best_match_index]
            # Duygu kontrolü
            gray_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
            faces = face_cascade.detectMultiScale(gray_frame, scaleFactor=1.1, minNeighbors=5, minSize=(30, 30))
            for (x, y, w, h) in faces:
                face_roi = gray_frame[y:y + h, x:x + w]
                resized_face = cv2.resize(face_roi, (48, 48), interpolation=cv2.INTER_AREA)
                normalized_face = resized_face / 255.0
                reshaped_face = normalized_face.reshape(1, 48, 48, 1)
                preds = model.predict(reshaped_face)[0]
                emotion_idx = preds.argmax()
                emotion = emotion_labels[emotion_idx]
                cv2.putText(frame, f"{name} - {emotion}", (x, y - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.9, (0, 0, 255), 2)
            current_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            data = {'Isim' : name ,'Duygu': emotion ,'Giris Zamani' : current_time}
            df = pd.concat([df, pd.DataFrame([data])], ignore_index=True)
            ser.write(f"{name},{emotion}\n".encode('utf-8'))
        else:
            ser.write(f"\n".encode('utf-8'))
    
    cv2.imshow('Video', frame)
    key = cv2.waitKey(3000)
    if key == ord('q'):
        break

df.to_csv("kayit.csv", sep=';', encoding='utf-8',index = False)

print(name,emotion)
video_capture.release()
cv2.destroyAllWindows()