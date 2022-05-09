
function scheduleNewAppointment() {
    const body = {};

    sendPostWebRequest(environment.webServerUrl + "/appointments", body, null);

    updateScheduleDisplay();
}

function updateScheduleDisplay() {
    const date = document.getElementById("dateForDisplay").value;
    
    sendGetWebRequest(environment.webServerUrl + "/appointments/by-date", "date=" + date, function () {
        if (this.readyState == 4 && this.status == 200) {
            document.getElementById("temp").innerHTML = this.responseText;
        }
    });
}

////// Main //////

// Initialize display date to today
// Must be formatted as yyyy-MM-dd
const initDateTime = new Date();
let initDateString = initDateTime.getFullYear() + "-";
if (initDateTime.getMonth() + 1 < 10) {
    initDateString += "0";
}
initDateString += (initDateTime.getMonth() + 1) + "-";
if (initDateTime.getDate() < 10) {
    initDateString += "0";
}
initDateString += initDateTime.getDate();

document.getElementById("dateForDisplay").value = initDateString;

const updateInterval = 60000; // 1 minute in milliseconds

setImmediateInterval(updateScheduleDisplay, updateInterval);
