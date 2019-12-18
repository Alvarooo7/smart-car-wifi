
var Socket = new WebSocket('ws://' + window.location.hostname + ':81');
Socket.onmessage = function (e) {
  var datos = e.data;
  var arrayDatos =datos.split("/");
  var distancia="0";
  var temperatura="0";
  var velocidad="0";
  var controlOcupado="0";
  distancia=arrayDatos[0];
  temperatura=arrayDatos[1];
  velocidad=arrayDatos[2];
  controlOcupado=arrayDatos[3];
  /*drawDial(distancia, '#ff5600');*/
  //Ultrasonico
  document.getElementById("contenidoDinamico").innerHTML="<p>Distancia con objeto</p>";
    if(distancia>20){

      document.getElementById("contenidoDinamico").innerHTML+="<h5 class='green-text'>"+distancia+" cm</h5>";
      document.getElementById("contenidoDinamico").innerHTML+="<i  class='material-icons small green-text'>done</i><br>";
      document.getElementById("contenidoDinamico").innerHTML+=" <a  class='green-text'>Sin Peligro</a>";
    }
    else if (distancia>14 && distancia<25){
      document.getElementById("contenidoDinamico").innerHTML+="<h5 class='yellow-text'>"+distancia+"  cm</h5>";
      document.getElementById("contenidoDinamico").innerHTML+="<i  class='material-icons small yellow-text'>warning</i><br>";
      document.getElementById("contenidoDinamico").innerHTML+="<a  class='yellow-text'>Alerta</a>";

    }else{
      document.getElementById("contenidoDinamico").innerHTML+="<h5 class='red-text'>"+distancia+" cm</h5>";
      document.getElementById("contenidoDinamico").innerHTML+="<i  class='material-icons small red-text'>highlight_off</i><br>";
      document.getElementById("contenidoDinamico").innerHTML+=" <a  class='red-text'>Peligro</a>";
    }


    //Temperatura
    document.getElementById("contenidoDinamicoTemperatura").innerHTML="<p>Temperatura Motor</p>";
    if(temperatura<25){

      document.getElementById("contenidoDinamicoTemperatura").innerHTML+="<h5 class='green-text'>"+temperatura+" C</h5>";
      document.getElementById("contenidoDinamicoTemperatura").innerHTML+="<i  class='material-icons small green-text'>done</i><br>";
      document.getElementById("contenidoDinamicoTemperatura").innerHTML+=" <a  class='green-text'>Sin Peligro</a>";
    }
    else if (temperatura>24 && temperatura<55){
      document.getElementById("contenidoDinamicoTemperatura").innerHTML+="<h5 class='yellow-text'>"+temperatura+" C</h5>";
      document.getElementById("contenidoDinamicoTemperatura").innerHTML+="<i  class='material-icons small yellow-text'>warning</i><br>";
      document.getElementById("contenidoDinamicoTemperatura").innerHTML+="<a  class='yellow-text'>Alerta</a>";

    }else{
      document.getElementById("contenidoDinamicoTemperatura").innerHTML+="<h5 class='red-text'>"+temperatura+" C</h5>";
      document.getElementById("contenidoDinamicoTemperatura").innerHTML+="<i  class='material-icons small red-text'>highlight_off</i><br>";
      document.getElementById("contenidoDinamicoTemperatura").innerHTML+=" <a  class='red-text'>Peligro</a>";
    }

    //Velocidad
    document.getElementById("contenidoDinamicoVelocidad").innerHTML="<p>Velocidad</p>";
    document.getElementById("contenidoDinamicoVelocidad").innerHTML+="<h5 class='green-text'>"+velocidad+"</h5>";
    document.getElementById("contenidoDinamicoVelocidad").innerHTML+="<i  class='material-icons small green-text'>keyboard_arrow_up</i><br>";
    document.getElementById("contenidoDinamicoVelocidad").innerHTML+=" <a  class='green-text'>cm/s</a>";
  
    //Control Ocupado?
    document.getElementById("control").value=controlOcupado;
    
};

















