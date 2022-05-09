
function sendGetWebRequest(url, args, callback) {
    let xhttp = new XMLHttpRequest();
    xhttp.open("GET", url + "?" + args, true);
    xhttp.send();
    xhttp.onreadystatechange = callback;
}

function sendPostWebRequest(url, body, callback) {
    let xhttp = new XMLHttpRequest();
    xhttp.open("POST", url, true);
    xhttp.setRequestHeader("Content-type", "application/json");
    xhttp.send(JSON.stringify(body));
    xhttp.onreadystatechange = callback;
}

// Creates an interval timer and also fires the callback function right away
function setImmediateInterval(callback, milliseconds) {
    callback();
    setInterval(callback, milliseconds);
}

console.log(Date());