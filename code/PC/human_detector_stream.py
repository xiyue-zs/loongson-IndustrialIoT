from flask import Flask, Response, jsonify, request, send_from_directory
from flask_cors import CORS
import cv2, threading, numpy as np, os, time

app = Flask(__name__)
CORS(app)

camera = cv2.VideoCapture("input.mp4")
if not camera.isOpened():
    raise RuntimeError("Failed to open video file.")

net = cv2.dnn.readNetFromCaffe("MobileNetSSD_deploy.prototxt", "MobileNetSSD_deploy.caffemodel")
CLASSES = ["background","aeroplane","bicycle","bird","boat","bottle","bus","car","cat",
           "chair","cow","diningtable","dog","horse","motorbike","person","pottedplant",
           "sheep","sofa","train","tvmonitor"]

last_count = 0
recording = False
video_writer = None
video_dir = "recordings"
os.makedirs(video_dir, exist_ok=True)

camera_lock = threading.Lock()

def generate_frames():
    global last_count, recording, video_writer
    while True:
        with camera_lock:
            ret, frame = camera.read()
        if not ret: 
            camera.set(cv2.CAP_PROP_POS_FRAMES, 0)  # 回到第一帧
            continue
        frame = cv2.resize(frame,(640,480))
        h, w = frame.shape[:2]
        blob = cv2.dnn.blobFromImage(cv2.resize(frame,(300,300)),0.007843,(300,300),127.5)
        net.setInput(blob)
        detections = net.forward()

        count = 0
        for i in range(detections.shape[2]):
            conf = float(detections[0,0,i,2])
            cls = int(detections[0,0,i,1])
            if conf > 0.6 and CLASSES[cls]=="person":
                count += 1
                sx, sy, ex, ey = (detections[0,0,i,3:7] * [w,h,w,h]).astype(int)
                if sy<ey and sx<ex:
                    roi=frame[sy:ey,sx:ex]
                    if roi.size:
                        gray=cv2.cvtColor(roi,cv2.COLOR_BGR2GRAY)
                        edges=cv2.Canny(gray,50,150)
                        edges_bgr=cv2.cvtColor(edges,cv2.COLOR_GRAY2BGR)
                        frame[sy:ey,sx:ex]=cv2.addWeighted(frame[sy:ey,sx:ex],0.7,edges_bgr,0.7,0)

        last_count = count
        if recording and video_writer:
            video_writer.write(frame)

        ret2, buf = cv2.imencode('.jpg',frame)
        yield (b'--frame\r\nContent-Type: image/jpeg\r\n\r\n'+buf.tobytes()+b'\r\n')

@app.route('/video_feed')
def video_feed(): return Response(generate_frames(), mimetype='multipart/x-mixed-replace; boundary=frame')
@app.route('/people_count')
def people_count(): return jsonify({"count": last_count})
@app.route('/is_recording')
def is_recording(): return jsonify({"recording": recording})

@app.route('/start_record', methods=['POST'])
def start_record():
    global recording, video_writer
    if not recording:
        files = sorted(os.listdir(video_dir))
        if len(files)>=5: os.remove(os.path.join(video_dir, files[0]))
        fname = f"rec_{int(time.time())}.mp4"
        video_writer = cv2.VideoWriter(os.path.join(video_dir, fname),
                                        cv2.VideoWriter_fourcc(*'mp4v'),20.0,(640,480))
        recording = True
        return jsonify({"status":"started","filename":fname})
    return jsonify({"status":"already"})

@app.route('/stop_record', methods=['POST'])
def stop_record():
    global recording, video_writer
    if recording:
        recording=False
        video_writer.release()
        video_writer=None
        return jsonify({"status":"stopped"})
    return jsonify({"status":"not"})

@app.route('/list_recordings')
def list_recordings():
    return jsonify(sorted(os.listdir(video_dir)))

@app.route('/recordings/<path:fname>')
def get_recording(fname):
    return send_from_directory(video_dir, fname)

@app.route('/delete_recording', methods=['POST'])
def delete_recording():
    data=request.get_json() or {}
    fname=data.get("filename","")
    p=os.path.join(video_dir,fname)
    if os.path.exists(p): os.remove(p)
    return jsonify({"status":"deleted"})

@app.after_request
def fix_iframe(r):
    r.headers['X-Frame-Options']='ALLOWALL'; return r

if __name__=='__main__':
    app.run(host='0.0.0.0', port=8500, threaded=False)