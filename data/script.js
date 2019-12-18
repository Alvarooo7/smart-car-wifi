

function validateUser(user){
  if(user.value.length < 1){
    console.log("not enough characters");
    error("rgb(189,87,87)");
  }else{
    error("rgb(87,189,130)");
    return true;
  }
}
function checkURL(url) {
	var res = url.match(/\.(jpeg|jpg|gif|png)$/) ;
    return(res!== null);
}
/*
function isValidURL(string) {
	var res = string.match(/(http(s)?:\/\/.)?(www\.)?[-a-zA-Z0-9@:%._\+~#=]{2,256}\.[a-z]{2,6}\b([-a-zA-Z0-9@:%_\+.~#?&//=]*)/g);
	if(res !== null && checkURL(string)){
		
		error("rgb(87,189,130)");
		return true;
	}else{
	
		error("rgb(189,87,87)");
	}
	
  };
*/

function validateEmail(email){
	const validation = /^\w+([\.-]?\w+)*@\w+([\.-]?\w+)*(\.\w{2,3})+$/;
	if(validation.test(email.value)){
		error("rgb(87,189,130)");
		return true;
	}else{
		 error("rgb(189,87,87)");
	}
}

function previousSlide(parent, nextForm){
	parent.classList.add("innactive");
	parent.classList.remove("active");
	nextForm.classList.add("active");
}


function nextSlide(parent, nextForm){
	parent.classList.add("innactive");
	parent.classList.remove("active");
	nextForm.classList.add("active");
}

function error(color){
  document.body.style.backgroundColor = color;
}

function animatedForm(){
	const arrows = document.querySelectorAll(".fa-arrow-right");
	
	 arrows.forEach(arrow => {
		   arrow.addEventListener("click", () => {
			  const input = arrow.previousElementSibling;
			  const parent = arrow.parentElement;
			  const nextForm = parent.nextElementSibling;
			 //check for validation
			 if(input.type === "text" && validateUser(input)){
			   nextSlide(parent, nextForm);
			 }else if(input.type === "email" && validateEmail(input)){
					nextSlide(parent, nextForm);
				}else if(input.type === "password" && validateUser(input)){
					nextSlide(parent, nextForm);
				}/* else if(input.type === "url" && isValidURL(input.value)){
					nextSlide(parent, nextForm);
				  }*/
					else{
						parent.style.animation = "shake 0.5s ease";
					}
					//get rid of animation
				  parent.addEventListener(`animationend`, () =>{
					  parent.style.animation = "";
				  })
		   });                        
   });
   const arrowsBack = document.querySelectorAll(".fa-arrow-left");
   arrowsBack.forEach(arrow2 => {
	  arrow2.addEventListener("click", () => {
		 const input = arrow2.previousElementSibling;
		 const parent = arrow2.parentElement;
		 const nextForm = parent.nextElementSibling;
		 const previous=parent.previousElementSibling
		 previousSlide(parent, previous);
		//check for validation
	   
			   parent.style.animation = "shake 0.5s ease";
  
			 //get rid of animation
			 parent.addEventListener(`animationend`, () =>{
				 parent.style.animation = "";
			 })
	  });                        
  });		
  
  }
animatedForm();