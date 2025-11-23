$(document).ready(function() {
    $('#trades').dataTable({"bFilter": false,"bInfo": false, "bPaginate": false, "aaSorting": [[0,'desc']]});
    $('#history_analysis').dataTable({"bFilter": false,"bInfo": false, "bPaginate": false});
    $('#tradesh').dataTable({
        "bInfo": false,
        "bPaginate": false,
        "aaSorting": [[4,'desc']]
        });
} );
