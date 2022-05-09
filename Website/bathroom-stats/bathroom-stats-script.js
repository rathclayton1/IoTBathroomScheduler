
const particleBaseUrl = "https://api.particle.io/v1/devices/" + environment.insideBathroomArgonDeviceId;
const displayDecimalPlaces = 2;

function updateInUseStatus() {
    sendGetWebRequest(particleBaseUrl + "/bathroomInUse", "access_token=" + environment.insideBathroomArgonAccessToken, function () {
        if (this.readyState == 4 && this.status == 200) {
            const resp = JSON.parse(this.responseText);

            const displayInUseParagraph = document.getElementById("display-in-use");

            if (resp.result == true) {
                displayInUseParagraph.innerHTML = "In use";
                displayInUseParagraph.classList.add("in-use");
                displayInUseParagraph.classList.remove("not-in-use");
            } else if (resp.result == false) {
                displayInUseParagraph.innerHTML = "Not in use";
                displayInUseParagraph.classList.remove("in-use");
                displayInUseParagraph.classList.add("not-in-use");
            }
        }
    });
}

function updateTemperature() {
    sendGetWebRequest(particleBaseUrl + "/temperatureC", "access_token=" + environment.insideBathroomArgonAccessToken, function () {
        if (this.readyState == 4 && this.status == 200) {
            const resp = JSON.parse(this.responseText);
            if (resp.result != null) {
                const toDisplay = resp.result.toFixed(displayDecimalPlaces);
                document.getElementById("display-temperature").innerHTML = toDisplay + "&deg; C";
            }
            
        }
    });
}

function updateHumidity() {
    sendGetWebRequest(particleBaseUrl + "/humidity", "access_token=" + environment.insideBathroomArgonAccessToken, function () {
        if (this.readyState == 4 && this.status == 200) {
            const resp = JSON.parse(this.responseText);
            if (resp.result != null) {
                const toDisplay = resp.result.toFixed(displayDecimalPlaces);
                document.getElementById("display-humidity").innerHTML = toDisplay + "%";
            }
        }
    });
}

function updateCurrentBathroomAppointment() {
    sendGetWebRequest(environment.webServerUrl + "/appointments/current", "", function () {
        if (this.readyState == 4 && this.status == 200) {
            const displayCurrentAppointmentParagraph = document.getElementById("display-current-appointment");
            
            if (this.responseText == "") { // this is what is returned when no such appointment exists
                displayCurrentAppointmentParagraph.innerHTML = "No one is scheduled to use the bathroom right now."
            } else {
                const resp = JSON.parse(this.responseText);

                displayCurrentAppointmentParagraph.innerHTML = 
                    resp.personName + " is scheduled to use the bathroom starting at " +
                    resp.startDateTime + " for " + resp.numMinutes + " minutes.";
            }
        }
    });
}

function updateNextBathroomAppointment() {
    sendGetWebRequest(environment.webServerUrl + "/appointments/next", "", function () {
        if (this.readyState == 4 && this.status == 200) {
            const displayNextAppointmentParagraph = document.getElementById("display-next-appointment");

            if (this.responseText == "") { // this is what is returned when no such appointment exists
                displayNextAppointmentParagraph.innerHTML = "There are no upcoming appointments."
            } else {
                const resp = JSON.parse(this.responseText);

                displayNextAppointmentParagraph.innerHTML = 
                    resp.personName + " is scheduled to use the bathroom starting at " +
                    resp.startDateTime + " for " + resp.numMinutes + " minutes.";
            }
        }
    });
}

////// Main //////

const updateInterval = 60000; // 1 minute in milliseconds

setImmediateInterval(updateInUseStatus, updateInterval);
setImmediateInterval(updateTemperature, updateInterval);
setImmediateInterval(updateHumidity, updateInterval);
setImmediateInterval(updateCurrentBathroomAppointment, updateInterval);
setImmediateInterval(updateNextBathroomAppointment, updateInterval);
