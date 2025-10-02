from flask import Flask, request, jsonify
import joblib
import numpy as np

app = Flask(__name__)

# Load model + scaler
scaler = joblib.load("../ml/scaler.pkl")
model = joblib.load("../ml/model.pkl")

@app.route("/predict", methods=["POST"])
def predict():
    data = request.json
    X = np.array([[
        data["ax"], data["ay"], data["az"],
        data["gx"], data["gy"], data["gz"],
        data["ir_value"]
    ]])
    X_scaled = scaler.transform(X)
    pred = model.predict(X_scaled)[0]

    # Console print instead of dashboard
    state = "DROWSY" if pred == 1 else "AWAKE"
    print(f"Prediction: {state} | Input: {data}")

    return jsonify({"prediction": int(pred)})

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=True)
