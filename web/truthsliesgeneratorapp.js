const truthsliesgeneratorapp = {};

truthsliesgeneratorapp.GeneratorApp = function(generatorService, ctors, data) {
    this.generatorService = generatorService;
    this.ctors = ctors;
    this.data = data;
};

truthsliesgeneratorapp.GeneratorApp.prototype.generate =
        function(truthsCount = 1, liesCount = 0) {
    let request = new this.ctors.GenerateRequest();
    request.setTruthsCount(truthsCount);
    request.setLiesCount(liesCount);
    let config = request.addConfigs();
    config.setTemplateString(
            'On {date}, I solved the 3x3x3 Rubik\'s Cube in exactly {time}.');
    config.setClassName('CubingStatementGenerator');
    this.data.forEach(function(entry) {
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
                .addClass('p-3 mb-2 bg-secondary text-white')
                .attr('data-truth', statement.getTruth())
                .text(index + statement.getStatement())));
};

truthsliesgeneratorapp.GeneratorApp.prototype.revealTruth =
        function() {
    $('#input-row').nextAll('div.row').children().each(function() {
        $(this).removeClass('bg-secondary');
        if ($(this).attr('data-truth') == "true") {
            $(this).addClass('bg-primary');
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
