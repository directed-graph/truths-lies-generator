const truthsliesgeneratorapp = {};

truthsliesgeneratorapp.GeneratorApp = function(generatorService, ctors, getData) {
    this.generatorService = generatorService;
    this.ctors = ctors;
    this.getData = getData;
};

truthsliesgeneratorapp.GeneratorApp.prototype.generate =
        async function(truthsCount = 1, liesCount = 0) {
    let sheetId = $('#spreadsheet-id').val();
    if (!sheetId) {
        return $('#configure-btn').click();
    }

    let data;
    try {
        data = await this.getData(sheetId, '1:1000');
    } catch(err) {
        let message = 'Failed to access.';
        if (typeof(err.result.error.message) !== 'undefined') {
            message = err.result.error.message;
        }
        $('#spreadsheet-id').val(
            message + ' (' + $('#spreadsheet-id').val() + ')');
        return $('#configure-btn').click();
    }

    let request = new this.ctors.GenerateRequest();
    request.setTruthsCount(truthsCount);
    request.setLiesCount(liesCount);
    let config = request.addConfigs();
    config.setTemplateString(data[0].template_string);
    config.setClassName(data[0].class_name);
    data.forEach(function(entry) {
        let argument = config.addArguments();
        let dateValue = new this.ctors.ValueMap.Value();
        dateValue.setStringValue(entry.date);
        argument.getValuesMap().set('date', dateValue);
        let timeValue = new this.ctors.ValueMap.Value();
        timeValue.setDoubleValue(entry.time);
        argument.getValuesMap().set('time', timeValue);
    }, this);
    var that = this;
    this.generatorService.generate(
        request, {},
        function(err, response) {
            if (err) {
                console.log('err: ', err, 'request: ', request);
            } else {
                response.getStatementsList().forEach(that.writeStatement);
            }
        });
};

truthsliesgeneratorapp.GeneratorApp.prototype.writeStatement =
        function(statement, index, array) {
    if (array) {
        index = String(array.length - index) + '. '
    } else {
        index = '';
    }
    $('#input-row').after(
        $('<div/>').addClass('row').append(
            $('<div/>')
                .addClass('col p-3 mb-2 bg-dark text-white')
                .attr('data-truth', statement.getTruth())
                .text(index + statement.getStatement())));
};

truthsliesgeneratorapp.GeneratorApp.prototype.revealTruth =
        function() {
    $('#input-row').nextAll('div.row').children().each(function() {
        $(this).removeClass('bg-dark');
        if ($(this).attr('data-truth') == "true") {
            $(this).addClass('bg-success');
        } else {
            $(this).addClass('bg-danger');
        }
    });
};

truthsliesgeneratorapp.GeneratorApp.prototype.clear =
        function() {
    $('#input-row').nextAll('div.row').children().remove();
};

truthsliesgeneratorapp.GeneratorApp.prototype.load = function() {
    var that = this;
    $(document).ready(function() {
        if (typeof(Storage) !== 'undefined') {
            $('#spreadsheet-id').val(localStorage.getItem('spreadsheet-id'));
            $('#save-btn').bind('click', () =>
                localStorage.setItem('spreadsheet-id',
                                     $('#spreadsheet-id').val()));
        }
        $('#generate-btn').bind('click', function(e) {
            that.generate(
                parseInt($('#truths-count').val()) || 0,
                parseInt($('#lies-count').val()) || 0,
            );
            return false;
        });
        $('#reveal-btn').bind('click', that.revealTruth);
        $('#clear-btn').bind('click', that.clear);
    });
};

module.exports = truthsliesgeneratorapp;
