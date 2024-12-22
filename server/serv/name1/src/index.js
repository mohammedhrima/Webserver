document.addEventListener("DOMContentLoaded", function () {
  const navLinks = document.querySelectorAll("nav a");
  const pages = document.querySelectorAll(".page");
  const themeToggle = document.getElementById("theme-toggle");

  function hidePages() {
    pages.forEach(page => page.style.display = "none");
  }

  function showPage(pageId) {
    hidePages();
    const page = document.getElementById(pageId);
    if (page) {
      page.style.display = "block";
    }
  }

  showPage("home");
  navLinks.forEach(link => {
    link.addEventListener("click", function (event) {
      event.preventDefault();
      const pageId = link.getAttribute("data-page");
      showPage(pageId);
    });
  });

  let isDarkMode = localStorage.getItem("theme") === "dark";
  function updateTheme() {
    if (isDarkMode) {
      document.body.classList.add("dark");
      themeToggle.textContent = "Switch to Light Mode";
    } else {
      document.body.classList.remove("dark");
      themeToggle.textContent = "Switch to Dark Mode";
    }
    localStorage.setItem("theme", isDarkMode ? "dark" : "light");
  }

  themeToggle.addEventListener("click", function () {
    isDarkMode = !isDarkMode;
    updateTheme();
  });

  updateTheme();
});
