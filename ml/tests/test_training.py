import joblib
import numpy as np

def test_model_prediction():
    scaler = joblib.load("scaler.pkl")
    model = joblib.load("model.pkl")

    sample = np.array([[0.0, 0.1, 9.8, 0.01, 0.02, 0.01, 600]])  # awake sample
    X = scaler.transform(sample)
    pred = model.predict(X)

    assert pred[0] in [0,1], "❌ Model prediction invalid"
    print("✅ Model test passed, prediction:", pred[0])
