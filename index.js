/* eslint-disable  func-names */
/* eslint-disable  no-console */

const Alexa = require('ask-sdk');
const AWS = require('aws-sdk');
// Replace the text in this endpoint with your AWS IoT Thing endpoint from the Interact section.
const iotData = new AWS.IotData({ endpoint: "a3kfcsv9dra7fl-ats.iot.us-west-2.amazonaws.com" });

const GetNewFactHandler = {
  canHandle(handlerInput) {
    const request = handlerInput.requestEnvelope.request;
    return request.type === 'LaunchRequest'
      || (request.type === 'IntentRequest'
        && request.intent.name === 'FarmGateIntent');
  },
  async handle(handlerInput) {

    time1 = await getShadowTimestamp('farm-gate-1');
    time2 = await getShadowTimestamp('farm-gate-2');
    const params = {
        topic: '$aws/things/farm-gate-1/shadow/update',
        payload: `{ "state": { "desired": {}}}`
      }
    console.log('farm ready to publish');
    iotData.publish(params, (err, res) => {
      if (err)
        console.log(err);
      else
        console.log(res);
    });
    
    const factArr = data;
    const factIndex = Math.floor(Math.random() * factArr.length);
    const resultAction = factArr[time1 > time2 ? 0 : 1];
    const speechOutput = ALEXA_FARM_RESPONSE_MESSAGE + resultAction;

        return handlerInput.responseBuilder
          .speak(speechOutput)
          .withSimpleCard(SKILL_NAME, resultAction)
          .getResponse();
      },
};

const getShadowTimestamp = name => {
  return new Promise((resolve, reject) => {
    iotData.getThingShadow({ thingName: name }, (err, data) => {
      if (err) reject(err)
      else resolve(JSON.parse(data.payload).metadata.desired.reading.timestamp)
    })
  })
}

const HelpHandler = {
  canHandle(handlerInput) {
    const request = handlerInput.requestEnvelope.request;
    return request.type === 'IntentRequest'
      && request.intent.name === 'AMAZON.HelpIntent';
  },
  handle(handlerInput) {
    return handlerInput.responseBuilder
      .speak(HELP_MESSAGE)
      .reprompt(HELP_REPROMPT)
      .getResponse();
  },
};

const ExitHandler = {
  canHandle(handlerInput) {
    const request = handlerInput.requestEnvelope.request;
    return request.type === 'IntentRequest'
      && (request.intent.name === 'AMAZON.CancelIntent'
        || request.intent.name === 'AMAZON.StopIntent');
  },
  handle(handlerInput) {
    return handlerInput.responseBuilder
      .speak(STOP_MESSAGE)
      .getResponse();
  },
};

const SessionEndedRequestHandler = {
  canHandle(handlerInput) {
    const request = handlerInput.requestEnvelope.request;
    return request.type === 'SessionEndedRequest';
  },
  handle(handlerInput) {
    console.log(`Session ended with reason: ${handlerInput.requestEnvelope.request.reason}`);

    return handlerInput.responseBuilder.getResponse();
  },
};

const ErrorHandler = {
  canHandle() {
    return true;
  },
  handle(handlerInput, error) {
    console.log(`Error handled: ${error.message}`);

    return handlerInput.responseBuilder
      .speak('Sorry, an error occurred.')
      .reprompt('Sorry, an error occurred.')
      .getResponse();
  },
};

const SKILL_NAME = 'Farm';
const ALEXA_FARM_RESPONSE_MESSAGE = 'Signal sent. ';
const HELP_MESSAGE = 'Available words for Farm are: Gate and Environment';
const HELP_REPROMPT = 'What can I help you with?';
const STOP_MESSAGE = 'Goodbye!';

const data = [
  'MOST RECENT ACTION: ENTRY',
  'MOST RECENT ACTION: EXIT'
];

const skillBuilder = Alexa.SkillBuilders.standard();

exports.handler = skillBuilder
  .addRequestHandlers(
    GetNewFactHandler,
    HelpHandler,
    ExitHandler,
    SessionEndedRequestHandler
  )
  .addErrorHandlers(ErrorHandler)
  .lambda();
