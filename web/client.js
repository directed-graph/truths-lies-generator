
const {GenerateRequest, GenerateResponse, ValueMap} =
    require('./truths_lies_generator_pb.js');
const {TruthsLiesGeneratorServiceClient} =
    require('./truths_lies_generator_grpc_web_pb.js');
const {GeneratorApp} = require('./truthsliesgeneratorapp.js');

const endpoint = 'https://endpoint.truths-lies-generator.everchanging.app';

const gSheetsApiKey = 'API_KEY';
const gSheetsApiUrl = 'https://sheets.googleapis.com/$discovery/rest?version=v4';

gapi.load('client', function() {
    gapi.client.setApiKey(gSheetsApiKey);
    gapi.client.load(gSheetsApiUrl)
        .then(null,
            (err) => console.error('Error loading GAPI client', err));
});

gSheetData = async function(sheetId, range) {
    let data = new Array();
    try {
        let rawData = await gapi.client.sheets.spreadsheets.values.get({
            spreadsheetId: sheetId,
            range: range,
        });
        let headers = rawData.result.values[0];
        for (let i = 1; i < rawData.result.values.length; ++i) {
            let dataPoint = {};
            for (let j = 0; j < headers.length; ++j) {
                dataPoint[headers[j]] = rawData.result.values[i][j];
            }
            data.push(dataPoint);
        }
    } catch(err) {
        console.error('Error getting data', err);
    } finally {
        return data;
    }
};

let generatorService = new TruthsLiesGeneratorServiceClient(endpoint);
let generatorApp = new GeneratorApp(
    generatorService=generatorService,
    ctors={
        GenerateRequest: GenerateRequest,
        GenerateResponse: GenerateResponse,
        ValueMap: ValueMap,
    },
    getData=gSheetData,
);

generatorApp.load();
