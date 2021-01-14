
const {GenerateRequest, GenerateResponse, ValueMap} =
    require('./truths_lies_generator_pb.js');
const {TruthsLiesGeneratorServiceClient} =
    require('./truths_lies_generator_grpc_web_pb.js');
const {GeneratorApp} = require('./truthsliesgeneratorapp.js');

const servicePort = '8080';
const dataSize = 10;

let data = Array.from({length: dataSize}, () => ({
    date: '2020-10-10T12:12:12-0700',
    time: Math.random() * 100,
}));

let generatorService = new TruthsLiesGeneratorServiceClient(
    'http://' + window.location.hostname + ':' + servicePort);
let generatorApp = new GeneratorApp(
    generatorService=generatorService,
    ctors={
        GenerateRequest: GenerateRequest,
        GenerateResponse: GenerateResponse,
        ValueMap: ValueMap,
    },
    data=data,
);

generatorApp.load();
