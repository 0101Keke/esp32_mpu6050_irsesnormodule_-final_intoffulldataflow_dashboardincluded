# train_quick.py
from pip import pandas as pd
from pip import numpy as np
import sys
sys.path.append('C:\Users\kekel\AppData\Local\Programs\Python\Python313\Lib\site-packages')
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
from sklearn.ensemble import RandomForestClassifier
from pip import joblib

df = pd.read_csv("data/mpu_ir_log.csv")

# ensure columns: accel_x etc... if different, adapt column names
if 'drowsy_label' not in df.columns:
    print("No drowsy_label column found. Aborting.")
    raise SystemExit

X = df[['accel_x','accel_y','accel_z','gyro_x','gyro_y','gyro_z','ir_raw']].values
y = df['drowsy_label'].values  # 0 = awake, 1 = drowsy

scaler = StandardScaler()
Xs = scaler.fit_transform(X)

X_train, X_test, y_train, y_test = train_test_split(Xs, y, test_size=0.2, random_state=42, stratify=y)

clf = RandomForestClassifier(n_estimators=200, random_state=42)
clf.fit(X_train, y_train)

print("Train acc:", clf.score(X_train,y_train))
print("Test acc:", clf.score(X_test,y_test))

joblib.dump(scaler, "ml/scaler.pkl")
joblib.dump(clf, "ml/model.pkl")
print("Saved scaler + model to ml/")
