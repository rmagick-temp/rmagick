// Alternate gray/white rows in all the tables having the class "striped"
function stripeTables() {
    if (!document.getElementsByTagName) {
        return false;
    }
    var tables = document.getElementsByTagName("table");
    for (var i = 0; i < tables.length; i++) {
        if (tables[i].className == "striped") {
            var gray = true;
            var rows = tables[i].getElementsByTagName("tr");
            for (var j = 0; j < rows.length; j++) {
                if (gray) {
                    rows[j].style.backgroundColor = "#f0f0f0";
                    gray = false;
                }
                else {
                    gray = true;
                }
            }
        }
    }
}

