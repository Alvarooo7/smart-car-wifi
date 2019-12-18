// Your web app's Firebase configuration
var firebaseConfig = {
    apiKey: "XXXXXXXXXXXXXXXXXXXGGGGGGGGGGGGGGG",
    authDomain: "fir-GG-GG.fXXXXaseGG.ORG",
    databaseURL: "https://fir-GGG-FGRGGG.GGGG.com",
    projectId: "fir-GGG-GGG",
    storageBucket: "",
};
// Initialize Firebase
firebase.initializeApp(firebaseConfig);

function getId(id) {
    return document.getElementById(id).value;
}

function arrayJSON(name, email) {
    var data = {
        name: name,
        email: email

    };
    return data;
}

let contadorLog = 0;


function inputsTask(id, result) {
    return document.getElementById(id).value = result;

}

function htmlnombreUsuario(usuario) {
    var arrayDatos = usuario.split(" ");
    return "Hola " + arrayDatos[0];
}

function htmlTable(id, DatosChoque) {
    return '<tr><th scope="row">' + id + '</th><td>' + DatosChoque + '</td></tr>';
}

function innerHTML(id, result) {
    return document.getElementById(id).innerHTML = result;
}

function innerAddHTML(id, result) {
    return document.getElementById(id).innerHTML += result;
}

function WatchTask() {
    var task = firebase.database().ref("task/1/");
    task.on("child_added", function(data) {
        var taskValue = data.val();
        //console.log(taskValue);
        var result = htmlnombreUsuario(taskValue);
        innerHTML("tituloUser", result);
    });
    //return document.getElementById(id).value=result;

}

function WatchTaskTable() {
    contadorLog = 0;
    var task = firebase.database().ref("log/crack/");
    task.on("child_added", function(data) {
        var taskValue = data.val();
        //console.log(taskValue);
        var result = htmlTable(++contadorLog, taskValue);
        innerAddHTML("loadLog", result);
    });
    //return document.getElementById(id).value=result;

}



function insertTask() {
    var id = 1;
    var name = getId("name");
    var email = getId("email");
    //console.log(name+"/"+email);
    if (name.lenght == 0 || email.lenght == 0) {
        alert("Parametros Vacio");
    } else {
        var arrayData = arrayJSON(name, email);
        //console.log(arrayData);
        var task = firebase.database().ref("task/" + id);
        task.set(arrayData);
        alert("Datos Guardados");
        inputsTask("name", "");
        inputsTask("email", "");

    }

}