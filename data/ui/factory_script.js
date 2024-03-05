async function get_value(parameter) {
    let input = document.getElementById(parameter);
    let response = await fetch("/configs/" + parameter)
    let resp_value = await response.text();
    let resp_end = `${resp_value}`
    if (resp_end.includes("Cannot GET") == true) {
      input.value = '';
    } else {
      input.value = resp_end;
    }
  }
  async function set_red() {
    document.getElementById("resetButton").style.backgroundColor =
      "#FF0000";
  }
  function setStyle(objId, propertyObject) {
    var elem = document.getElementById(objId);
    for (var property in propertyObject)
      elem.style[property] = propertyObject[property];
  }
  async function get_value_with_delay(parameter) {
    let oldValue = document.getElementById(parameter).value;
    setTimeout(() => {
      get_value(parameter);
    }, 1000);
    let newValue = document.getElementById(parameter).value;
    if (oldValue === newValue) {
      setStyle(parameter, { "border-color": "#4bfb00" });
    } else {
      setStyle(parameter, { "border-color": "#702f2f" });
    }
  }
  function submitMessage() {
    setTimeout(function () {
      document.location.reload(false);
    }, 500);
  }
async function funonload() {
    get_value("test_url");
    get_value("sn");
    get_value("mqtt_token");
    get_value("firmware");
    get_value("hardware");
}
let checked = 1;

function check(event) {
  if (event.target.parentNode[1].value == "✔") {
    if (checked == 1) {
      event.target.parentNode[1].value = "Save";
    }
  } else {
    if (checked == 1) {
      event.target.parentNode[1].value = "✔";
      event.target.parentNode[1].style.color = "rgb(9, 255, 0)";
    }
  }
}
