/****
Name: IoTBathroomScheduler - WebServer
Author: Dylan Schulz, Clayton Rath, Sean Stille, Justin Vang
Description: Keeps track of bathroom appointments and allows
  this information to be queried in various ways.
Bugs: 
Reflection: 
****/

////// Setup //////

'use strict';

const express = require('express');
let bodyParser = require('body-parser');
const cors = require('cors');

const app = express();
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: true}));
app.use(cors());

const {Datastore} = require('@google-cloud/datastore');

const datastore = new Datastore({
  projectId: 'iotbathroomscheduler',
});

////// Data handling //////

async function getAllAppointments() {
  const query = datastore
    .createQuery('appointment')
    .order('startDateTime');

  const [entities] = await datastore.runQuery(query);
  
  return entities;
}

async function getAppointmentsByDate(date) {
  const allAppointments = await getAllAppointments();

  // Filter the list to only appointments starting on the given date
  const appointments = allAppointments.filter(
    function (entity) {
      const startDateTime = new Date(entity.startDateTime);
      return startDateTime.getUTCFullYear() == date.getUTCFullYear() &&
             startDateTime.getUTCMonth() == date.getUTCMonth() &&
             startDateTime.getUTCDay() == date.getUTCDay();
    }
  );

  return appointments;
}

async function getCurrentAppointment() {
  const allAppointments = await getAllAppointments();
  
  const msPerMin = 60000;

  // Find the first appointment for which the start is before now or is now
  // and the end is after now

  const appointment = allAppointments.find(
    function (entity) {
      const startDateTime = new Date(entity.startDateTime);
      // Get end time by adding the number of minutes converted to milliseconds
      const endDateTime = new Date(startDateTime.getTime() + entity.numMinutes * msPerMin);
      const now = new Date();
      // Compare using epoch time
      return startDateTime.getTime() <= now.getTime() &&
             now.getTime() < endDateTime.getTime();
    }
  );

  if (appointment === undefined) {
    return null;
  }

  appointment.epochTime = appointment.startDateTime.getTime();

  return appointment;
}

async function getNextAppointment() {
  const allAppointments = await getAllAppointments();

  // Find the first appointment for which the start is after now

  const appointment = allAppointments.find(
    function (entity) {
      const startDateTime = new Date(entity.startDateTime);
      const now = new Date();
      // Compare using epoch time
      return now.getTime() < startDateTime.getTime();
    }
  );

  if (appointment === undefined) {
    return null;
  }

  appointment.epochTime = appointment.startDateTime.getTime();

  return appointment;
}

// Returns true or false to indicate whether there is an existing
// appointment that conflicts with the given appointment criteria
async function isThereAConflictingAppointment(newStartDateTime, newNumMinutes) {
  const allAppointments = await getAllAppointments();
  
  const msPerMin = 60000;
  // Get end time by adding the number of minutes converted to milliseconds
  const newEndDateTime = new Date(newStartDateTime.getTime() + newNumMinutes * msPerMin);

  // Check if there is an existing appointment for which one of the following is true,
  // and if so, returns true indicating there is a conflicting appointment, else return false:
  // newStart is between existingStart and existingEnd
  // newEnd is between existingStart and existingEnd
  // (newStart is before existingStart) AND (newEnd is after existingEnd)

  return allAppointments.some(
    function (entity) {
      const existingStartDateTime = new Date(entity.startDateTime);
      // Get end time by adding the number of minutes converted to milliseconds
      const existingEndDateTime = new Date(existingStartDateTime.getTime() + entity.numMinutes * msPerMin);
      // Compare using epoch time
      return (existingStartDateTime.getTime() <= newStartDateTime.getTime() && newStartDateTime.getTime() < existingEndDateTime.getTime()) ||
             (existingStartDateTime.getTime() < newEndDateTime.getTime() && newEndDateTime.getTime() <= existingEndDateTime.getTime()) ||
             (newStartDateTime.getTime() <= existingStartDateTime.getTime() && existingEndDateTime.getTime() <= existingEndDateTime.getTime());
    }
  );
}

async function addAppointment(startDateTime, numMinutes, personName) {
  const entity = {
    key: datastore.key('appointment'),
    data: {
      startDateTime: startDateTime,
      numMinutes: numMinutes,
      personName: personName,
    },
  };

  return await datastore.save(entity);
}

////// HTTP request handling //////

// For the purposes of these endpoints, an appointment is an object with
// startDateTime as an ISO 8601 string,
// numMinutes as an integer, representing the length, and
// personName as a string, representing the person who has the appointment

// For the next and current endpoints, an appointment also includes
// epochTime, representing the startDateTime as ms since Jan 1, 1970

// Returns a JSON array of all appointments
app.get('/appointments', async (req, res) => {
  try {
    const appointments = await getAllAppointments();
    res.status(200).json(appointments).end();

  } catch(err) {
    console.error(`Error: ${err}`);
    res.status(500).end();
  }
});

// Takes in a JSON object that contains date,
// representing the desired date as an ISO 8601 string
// Returns a JSON array of all appointments on a given date
app.get('/appointments/by-date', async (req, res) => {
  try {
    const date = new Date(req.query.date);
    
    const appointments = await getAppointmentsByDate(date);
    res.status(200).json(appointments).end();
    
  } catch(err) {
    console.error(`Error: ${err}`);
    res.status(500).end();
  }
});

// Returns a JSON object of the current appointment, or
// null if there is no current appointment
app.get('/appointments/current', async (req, res) => {
  try {
    const appointment = await getCurrentAppointment();
    if (appointment === null) {
      res.status(200).end();
    } else {
      res.status(200).json(appointment).end();
    }
    
  } catch(err) {
    console.error(`Error: ${err}`);
    res.status(500).end();
  }
});

// Returns a JSON object of the next appointment that
// is not currently ongoing, or
// null if there are no upcoming appointments
app.get('/appointments/next', async (req, res) => {
  try {
    const appointment = await getNextAppointment();
    if (appointment === null) {
      res.status(200).end();
    } else {
      res.status(200).send(appointment).end();
    }

  } catch(err) {
    console.error(`Error: ${err}`);
    res.status(500).end();
  }
});

// Takes in a JSON object of an appointment and schedules it
// unless there is a conflicting appointment, in which case
// an error code is returned
app.post('/appointments', async (req, res) => {
  try {
    const startDateTime = new Date(req.body.startDateTime);

    if (await isThereAConflictingAppointment(startDateTime, req.body.numMinutes)) {
      res.status(400).end();
    } else {
      await addAppointment(startDateTime, req.body.numMinutes, req.body.personName);
      res.status(200).end();
    }

  } catch(err) {
    console.error(`Error: ${err}`);
    res.status(500).end();
  }
});

////// Start the server //////

const PORT = parseInt(process.env.PORT) || 8080;
app.listen(PORT, () => {
  console.log(`App listening on port ${PORT}`);
  console.log('Press Ctrl+C to quit.');
});

module.exports = app;
