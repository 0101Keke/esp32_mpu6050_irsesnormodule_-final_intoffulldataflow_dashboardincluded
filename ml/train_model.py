import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn.ensemble import RandomForestClassifier
from sklearn.preprocessing import StandardScaler
from sklearn.metrics import accuracy_score
import joblib

# Load data
df = pd.read_csv("../data/imu_ir_log.csv")

# Features and labels
X = df[["ax","ay","az","gx","gy","gz","ir_value"]]
y = df["drowsy_state"]

# Scale
scaler = StandardScaler()
X_scaled = scaler.fit_transform(X)

# Split
X_train, X_test, y_train, y_test = train_test_split(X_scaled, y, test_size=0.2, random_state=42)

# Train model
rf = RandomForestClassifier(n_estimators=100, random_state=42)
rf.fit(X_train, y_train)

# Evaluate
print("Accuracy:", accuracy_score(y_test, rf.predict(X_test)))

# Save
joblib.dump(scaler, "scaler.pkl")
joblib.dump(rf, "model.pkl")
