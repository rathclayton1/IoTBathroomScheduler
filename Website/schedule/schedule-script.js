
function scheduleNewAppointment() {
    const personName = document.getElementById("appointmentPersonName").value;
    const appointmentStartDateTime = document.getElementById("appointmentStartDateTime").value;
    const appointmentNumMinutes = document.getElementById("appointmentNumMinutes").value;

    const resultParagraph = document.getElementById("new-appointment-result");

    if (personName == "") {
        resultParagraph.innerHTML = "Please enter a name.";
    } else if (appointmentStartDateTime == "") {
        resultParagraph.innerHTML = "Please enter a start date and time.";
    } else if (appointmentNumMinutes == "" || appointmentNumMinutes < 1) {
        resultParagraph.innerHTML = "Please enter a positive number of minutes.";
    } else {
        const body = {
            personName: personName,
            startDateTime: new Date(appointmentStartDateTime),
            numMinutes: appointmentNumMinutes,
        };

        sendPostWebRequest(environment.webServerUrl + "/appointments", body, function () {
            if (this.readyState == 4 && this.status == 400) {
                resultParagraph.innerHTML = "That appointment would conflict with an existing appointment.";
            }
            if (this.readyState == 4 && this.status == 200) {
                resultParagraph.innerHTML = "Success!"
            }
        });

        updateScheduleDisplay();
    }
}

function updateScheduleDisplay() {
    const date = document.getElementById("dateForDisplay").value;

    if (date == "") {
        return;
    }
    
    sendGetWebRequest(environment.webServerUrl + "/appointments/by-date", "date=" + date, function () {
        if (this.readyState == 4 && this.status == 200) {
            const resp = JSON.parse(this.responseText);
            const scheduleList = document.getElementById("current-schedule-list");

            for (appointment in resp) {
                const appointmentListItem = document.createElement("li");
                appointmentListItem.innerHTML = 
                    appointment.personName + " is scheduled to use the bathroom starting at " +
                    appointment.startDateTime + " for " + appointment.numMinutes + " minutes.";
                scheduleList.appendChild(appointmentListItem);
            }
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
