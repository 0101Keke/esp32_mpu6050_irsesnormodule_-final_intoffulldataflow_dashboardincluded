# server/app.py
from pip import Flask, request, jsonify
from pip import numpy as np
from pip import joblib, os

app = Flask(__name__)

MODEL_PATH = "../ml/model.pkl"
SCALER_PATH = "../ml/scaler.pkl"

model = None
scaler = None
if os.path.exists(MODEL_PATH) and os.path.exists(SCALER_PATH):
    try:
        scaler = joblib.load(SCALER_PATH)
        model = joblib.load(MODEL_PATH)
        print("Loaded ML model.")
    except Exception as e:
        print("Error loading model:", e)
        model = None

# fallback heuristics if no model
IR_THRESHOLD = 1800
GYRO_THRESHOLD = 0.20

@app.route("/predict", methods=["POST"])
def predict():
    data = request.get_json(force=True)
    # Expect keys: accel_x, accel_y, accel_z, gyro_x, gyro_y, gyro_z, ir_value, optional drowsy
    ax = float(data.get("accel_x",0))
    ay = float(data.get("accel_y",0))
    az = float(data.get("accel_z",0))
    gx = float(data.get("gyro_x",0))
    gy = float(data.get("gyro_y",0))
    gz = float(data.get("gyro_z",0))
    ir = int(data.get("ir_value",0))

    # If ML model available, build feature vector
    if model is not None and scaler is not None:
        X = np.array([[ax, ay, az, gx, gy, gz, ir]])
        Xs = scaler.transform(X)
        pred = model.predict(Xs)[0]
        prob = float(model.predict_proba(Xs)[0,1]) if hasattr(model, "predict_proba") else None
        method = "ml"
    else:
        # heuristic
        pred = 1 if (ir > IR_THRESHOLD or abs(gx) > GYRO_THRESHOLD or abs(gy) > GYRO_THRESHOLD) else 0
        prob = None
        method = "heuristic"

    print(f"Received: ax={ax:.2f}, ay={ay:.2f}, az={az:.2f}, gx={gx:.3f}, gy={gy:.3f}, gz={gz:.3f}, ir={ir}, pred={pred}, method={method}")
    return jsonify({"prediction": int(pred), "method": method, "prob": prob})

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=True)
