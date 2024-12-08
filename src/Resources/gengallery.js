// Array of image names
const imgs = ["2502852653", "image2", "image3"]; // Replace with your actual file names

// Get the parent container where the gallery will be added
const galleryContainer = document.querySelector("#photo-gallery"); // Replace with the actual ID or class of your gallery container

// Loop through the imgs array and create the HTML structure for each image
imgs.forEach(imgName => {
    // Create the main img-container div
    const imgContainer = document.createElement("div");
    imgContainer.className = "img-container";

    // Create the imatte div
    const imatte = document.createElement("div");
    imatte.className = "imatte";

    // Create the inner div with the ID
    const innerDiv = document.createElement("div");
    innerDiv.id = imgName;

    // Create the img element
    const imgElement = document.createElement("img");
    imgElement.setAttribute("w", 600);
    imgElement.setAttribute("h", 400);
    imgElement.className = "thumb";
    imgElement.src = `../thumbs/${imgName}.jpg`;
    imgElement.alt = "&nbsp;";
    imgElement.setAttribute("onclick", `javascript:ShowImage('../imgs/${imgName}.jpg', '&nbsp;')`);

    // Append the img element to the inner div
    innerDiv.appendChild(imgElement);

    // Append the inner div to the imatte div
    imatte.appendChild(innerDiv);

    // Create the links div
    const linksDiv = document.createElement("div");
    linksDiv.className = "links";

    // Create the title div
    const titleDiv = document.createElement("div");
    titleDiv.className = "title";
    titleDiv.setAttribute("onclick", `javascript:ShowImage('../imgs/${imgName}.jpg', '&nbsp;')`);
    titleDiv.innerHTML = "&nbsp;"; // Add title content if needed

    // Append the title div to the links div
    linksDiv.appendChild(titleDiv);

    // Append imatte and links to the main img-container
    imgContainer.appendChild(imatte);
    imgContainer.appendChild(linksDiv);

    // Append the img-container to the gallery container
    galleryContainer.appendChild(imgContainer);
});
/* into the HTML
	<div id="photo-gallery"></div>
*/
