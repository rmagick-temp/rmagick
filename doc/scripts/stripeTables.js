// Alternate gray/white rows in all the tables having the class "striped"
function stripeTables() {
    if (!document.getElementsByTagName) {
        return false;
    }
    var tables = document.getElementsByTagName("table");
    for (var i = 0; i < tables.length; i++) {
        if (tables[i].className == "striped") {
            var odd = false;
            var rows = tables[i].getElementsByTagName("tr");
            for (var j = 0; j < rows.length; j++) {
                if (odd) {
                    rows[j].style.backgroundColor = "#f0f0f0";
                    odd = false;
                }
                else {
                    odd = true;
                }
            }
        }
    }
}

