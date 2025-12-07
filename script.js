const firebaseConfig = {
        apiKey: "YOUR_API_KEY",
        authDomain: "your_auth_domain",
        databaseURL:"YOUR_DB_URL",
        projectId: "YOUR_PROJECTID",
        storageBucket: "YOUR_STORAGEBUCKET", 
        messagingSenderId: "YOUR_MESSAGINGSENDERID",
        appId: "YOUR_APPID"
};
firebase.initializeApp(firebaseConfig);
const db = firebase.database();
const heartRateEl = document.getElementById("heartRate");
const pressureEl = document.getElementById("pressure");
const temperatureEl = document.getElementById("temperature");
const timestampEl = document.getElementById("timestamp");
const vitalsRef = db.ref("patients/patient1/vitals");
let latestKey = "";
// Format timestamp to readable date & time
function formatTimestamp(unixTime) {
  if (!unixTime) return "--";
  const date = new Date(unixTime);
  return date.toLocaleString();  // You can customize format if needed
}
// Show vitals and check for abnormalities
vitalsRef.orderByKey().limitToLast(1).on("child_added", (snapshot) => {
  const data = snapshot.val();
  if (data) {
    heartRateEl.textContent = `${data.heartRate ?? "--"} bpm`;
    pressureEl.textContent = `${data.pressure_mmHg?.toFixed(2) ?? "--"} mmHg`;
    temperatureEl.textContent = `${data.temperature_F?.toFixed(1) ?? "--"} °F`;
    timestampEl.textContent = data.timestamp ?? "--";
    checkAbnormal(data);
  }
});
const alertBox = document.getElementById("alertBox");
function checkAbnormal(data) {
  const alerts = [];
  if (data.heartRate < 60 || data.heartRate > 120) {
    alerts.push(`❤️ Heart rate abnormal: ${data.heartRate} bpm`);
  }
  if (data.temperature_F < 97 || data.temperature_F > 100.4) {
  alerts.push(`🌡 Temperature abnormal: ${data.temperature_F} °F`);
}
  if (data.pressure_mmHg < 735 || data.pressure_mmHg > 790) {
    alerts.push(`🌬 Pressure abnormal: ${data.pressure_mmHg} mmHg`);
  }
  if (alerts.length > 0) {
    alertBox.innerHTML = alerts.join("<br>");
    alertBox.classList.remove("hidden");
    alertBox.classList.add("critical");
  } else {
    alertBox.innerHTML = "";
    alertBox.classList.add("hidden");
    alertBox.classList.remove("critical");
  }
}

