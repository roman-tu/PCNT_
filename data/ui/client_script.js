let modalW = document.getElementsByClassName("popapp")
let btnInd = 1
let btn1 = 1
let btn2 = 2
let btn3 = 3
let btn4 = 4
let adr_on_btn
let xhr = new XMLHttpRequest(); // у конструктора нет аргументов

async function get_value(parameter) {
  let input = document.getElementById(parameter);
  let response = await fetch("/configs/" + parameter)
  let resp_value = await response.text();
  let resp_end = `${resp_value}`
  try {

    if (resp_end.includes("Cannot GET") == true) {
      input.value = '';
    } else {
      input.value = resp_end;
    }
  
  } catch (err) {
  
    if (resp_end.includes("Cannot GET") == true) {
      input.value = '';
    } else {
      input.value = resp_end;
    }
  
  }
  
}

async function get_text(parameter) {
    let textt = document.getElementById(parameter);
    let response = await fetch("/configs/" + parameter)
    let resp_value = await response.text();
    textt.style.color = "#fff"
    if (resp_value.includes("Cannot GET") == true) {
      textt.innerText = `No connection`
    } else {
      if (parameter == "firmware") {
        textt.innerText = `Firmware: ${resp_value}`
      }else if(parameter == "hardware"){
        textt.innerText = `Hardware: ${resp_value}`
      }else if(parameter == "sn"){
        textt.innerText = `SN: ${resp_value}`
      }else{
        textt.innerText = `${resp_value}`
      }
    }
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
  }, 500);
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
  get_value("url_button_1_pressed");
  get_value("url_button_1_released");
  get_value("url_button_2_pressed");
  get_value("url_button_2_released");
  // get_value("sn");
  get_value("place");
  get_value("mqtt_url");
  get_value("mqtt_token");
  get_value("mqtt_topic");
  get_value("attr_topic");
  get_value("auth_period");
  get_value("ssid");
  get_value("ssid_pass");
  get_value("user_name");
  get_value("user_password");
  get_value("ip_address");
  get_value("gateway");
  get_value("netmask");
  get_value("dns_1");
  get_value("dns_2");
  get_value("update_period");
  get_text("hardware");
  get_text("firmware");
  get_text("sn");
}



function check(event) {
      event.target.parentNode[1].value = "✔";
      event.target.parentNode[1].style.color = "rgb(9, 255, 0)";
      checked = 0
      checked_g_main_btn = checked
      setTimeout(()=>{
      event.target.parentNode[1].value = "Save";
      event.target.parentNode[1].style.color = "rgb(255, 255, 255)";
      checked = 1
      checked_g_main_btn = checked
    },500)
    if(checked == 1){
      checked -= 1
    }else{
      event.target.parentNode[2].style.display = "inline"
      check_wi(event)
    }
}
function check_wi(event) {
  if(modalW[0].style.display === "inline"){
    if(event.target.parentNode[2].className === "krasiva-animatsiya-knopki"){

        // event.target.parentNode[2].style.color = "rgb(255, 255, 255)";
        // event.target.parentNode[2].style.backgroundColor = "rgb(157, 0, 0)";
        event.target.parentNode[2].className = "";
 
    }else{
      event.target.parentNode[2].className = "krasiva-animatsiya-knopki";
    }
  }else{
    
    if(event.target.parentNode[1].value === "Save"){
      modalW[0].style.display = "inline"
      // event.target.parentNode[2].style.color = "rgb(255, 255, 255)";
      event.target.parentNode[2].style.backgroundColor = "rgba(18, 61, 192, 0.587)";
      event.target.parentNode[2].className = "krasiva-animatsiya-knopki";
      setTimeout(()=>{
        event.target.parentNode[2].className = " ";
        event.target.parentNode[2].style.color = "rgb(255, 255, 255)";
        event.target.parentNode[2].style.backgroundColor = "rgb(157, 0, 0,0)";
      },5000)
    }
  }
  if(event.target.name == "1"){
    btnInd = 1
  }else if(event.target.name == "2"){
    btnInd = 2
  }else if(event.target.name == "3"){
    btnInd = 3
  }else if(event.target.name == "4"){
    btnInd = 4
  }
  else{
    btnInd = 1
  }
  
}

function FUNCTIONS(event, place_id, param){
  check_wi(event, place_id)

}



async function submit_form(event){
  modalW[0].style.display = "none"
  event.preventDefault();
  if(btnInd == "1"){
    let action = `/test/`
    let name = `test_${btnInd}`
    let value = `1`
    xhr.open("GET", `${action}${name}=${btnInd}`, [true])
    xhr.send()
    xhr.onload = function() {
      if(xhr.status >= 200 && xhr.status < 300){
        adr_on_btn.className = "";
        adr_on_btn.style.backgroundColor = "rgba(0, 255, 0, 0.587)";
        adr_on_btn.className = "krasiva-animatsiya-knopki-good";
        setTimeout(()=>{
          adr_on_btn.className = " ";
          adr_on_btn.style.color = "rgb(255, 255, 255)";
          adr_on_btn.style.backgroundColor = "rgb(157, 0, 0,0)";
        },2000)
      } else {
        adr_on_btn.className = "";
        adr_on_btn.style.backgroundColor = "rgba(255, 0, 0, 0.587)";
        adr_on_btn.className = "krasiva-animatsiya-knopki-bad";
        setTimeout(()=>{
          adr_on_btn.className = " ";
          adr_on_btn.style.color = "rgb(255, 255, 255)";
          adr_on_btn.style.backgroundColor = "rgb(157, 0, 0,0)";
        },2000)
      }
    };
    
    xhr.onerror = function() { // происходит, только когда запрос совсем не получилось выполнить
      console.log(`Ошибка соединения`);
    };
    
    xhr.onprogress = function(event) { // запускается периодически
      // event.loaded - количество загруженных байт
      // event.lengthComputable = равно true, если сервер присылает заголовок Content-Length
      // event.total - количество байт всего (только если lengthComputable равно true)
    };
    btnInd = 1
   }else if(btnInd == "2"){
    let action = `/test/`
    let name = `test_${btnInd}`
    let value = `2`
    xhr.open("GET", `${action}${name}=${btnInd}`, [true])
    xhr.send()
    xhr.onload = function() {
      if(xhr.status >= 200 && xhr.status < 300){
        adr_on_btn.className = "";
        adr_on_btn.style.backgroundColor = "rgba(0, 255, 0, 0.587)";
        adr_on_btn.className = "krasiva-animatsiya-knopki-good";
        setTimeout(()=>{
          adr_on_btn.className = " ";
          adr_on_btn.style.color = "rgb(255, 255, 255)";
          adr_on_btn.style.backgroundColor = "rgb(157, 0, 0,0)";
        },2000)
      } else {
        adr_on_btn.className = "";
        adr_on_btn.style.backgroundColor = "rgba(255, 0, 0, 0.587)";
        adr_on_btn.className = "krasiva-animatsiya-knopki-bad";
        setTimeout(()=>{
          adr_on_btn.className = " ";
          adr_on_btn.style.color = "rgb(255, 255, 255)";
          adr_on_btn.style.backgroundColor = "rgb(157, 0, 0,0)";
        },2000)
      }
    };
    
    xhr.onerror = function() { // происходит, только когда запрос совсем не получилось выполнить
      console.log(`Ошибка соединения`);
    };
    
    xhr.onprogress = function(event) { // запускается периодически
      // event.loaded - количество загруженных байт
      // event.lengthComputable = равно true, если сервер присылает заголовок Content-Length
      // event.total - количество байт всего (только если lengthComputable равно true)
    };
    btnInd = 2
  }else if(btnInd == "3"){
    let action = `/test/`
    let name = `test_${btnInd}`
    let value = `3`
    xhr.open("GET", `${action}${name}=${btnInd}`, [true])
    xhr.send()
    xhr.onload = function() {
      if(xhr.status >= 200 && xhr.status < 300){
        adr_on_btn.className = "";
        adr_on_btn.style.backgroundColor = "rgba(0, 255, 0, 0.587)";
        adr_on_btn.className = "krasiva-animatsiya-knopki-good";
        setTimeout(()=>{
          adr_on_btn.className = " ";
          adr_on_btn.style.color = "rgb(255, 255, 255)";
          adr_on_btn.style.backgroundColor = "rgb(157, 0, 0,0)";
        },2000)
      } else {
        adr_on_btn.className = "";
        adr_on_btn.style.backgroundColor = "rgba(255, 0, 0, 0.587)";
        adr_on_btn.className = "krasiva-animatsiya-knopki-bad";
        setTimeout(()=>{
          adr_on_btn.className = " ";
          adr_on_btn.style.color = "rgb(255, 255, 255)";
          adr_on_btn.style.backgroundColor = "rgb(157, 0, 0,0)";
        },2000)
      }
    };
    
    xhr.onerror = function() { // происходит, только когда запрос совсем не получилось выполнить
      console.log(`Ошибка соединения`);
    };
    
    xhr.onprogress = function(event) { // запускается периодически
      // event.loaded - количество загруженных байт
      // event.lengthComputable = равно true, если сервер присылает заголовок Content-Length
      // event.total - количество байт всего (только если lengthComputable равно true)
    };
    btnInd = 3
  }else if(btnInd == "4"){
    let action = `/test/`
    let name = `test_${btnInd}`
    let value = `4`
    xhr.open("GET", `${action}${name}=${btnInd}`, [true])
    xhr.send()
    xhr.onload = function() {
      if(xhr.status >= 200 && xhr.status < 300){
        adr_on_btn.className = "";
        adr_on_btn.style.backgroundColor = "rgba(0, 255, 0, 0.587)";
        adr_on_btn.className = "krasiva-animatsiya-knopki-good";
        setTimeout(()=>{
          adr_on_btn.className = " ";
          adr_on_btn.style.color = "rgb(255, 255, 255)";
          adr_on_btn.style.backgroundColor = "rgb(157, 0, 0,0)";
        },2000)
      } else {
        adr_on_btn.className = "";
        adr_on_btn.style.backgroundColor = "rgba(255, 0, 0, 0.587)";
        adr_on_btn.className = "krasiva-animatsiya-knopki-bad";
        setTimeout(()=>{
          adr_on_btn.className = " ";
          adr_on_btn.style.color = "rgb(255, 255, 255)";
          adr_on_btn.style.backgroundColor = "rgb(157, 0, 0,0)";
        },2000)
      }
    };
    
    xhr.onerror = function() { // происходит, только когда запрос совсем не получилось выполнить
      console.log(`Ошибка соединения`);
    };
    
    xhr.onprogress = function(event) { // запускается периодически
      // event.loaded - количество загруженных байт
      // event.lengthComputable = равно true, если сервер присылает заголовок Content-Length
      // event.total - количество байт всего (только если lengthComputable равно true)
    };
    btnInd = 4
  }
}
function closeMW(){
  modalW[0].style.display = "none"
}

// axios
//   .get('foo.example')
//   .then((response) => {})
//   .catch((error) => {
//     console.log(error); //Logs a string: Error: Request failed with status code 404
//   });

async function checking_serv_code(event){
  if(event.target.parentNode[1].value === "Save"){
    modalW[0].style.display = "inline"
    // event.target.parentNode[2].style.color = "rgb(255, 255, 255)";
    event.target.parentNode[2].style.backgroundColor = "rgba(18, 61, 192, 0.587)";
    event.target.parentNode[2].className = "krasiva-animatsiya-knopki";

    if(event.target.name == "1"){
      btnInd = 1
      adr_on_btn = document.getElementById(btnInd)
    }else if(event.target.name == "2"){
      btnInd = 2
      adr_on_btn = document.getElementById(btnInd)
    }else if(event.target.name == "3"){
      btnInd = 3
      adr_on_btn = document.getElementById(btnInd)
    }else if(event.target.name == "4"){
      btnInd = 4
      adr_on_btn = document.getElementById(btnInd)
    }
    setTimeout(()=>{
      fetch(`/test/?test_${btnInd}=${btnInd}`)
        .then((res) => {
          console.log(res)
            
        })
        .catch((e) => {
            console.log('Error: ' + e.message);
            console.log(e.response);
        });
      console.log("ВСЁ ГУД МЭН")
    }, 1000)
}}