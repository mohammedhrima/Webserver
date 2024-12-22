document.addEventListener("DOMContentLoaded", function () {
    const form = document.getElementById('user-form');
    const responseContainer = document.getElementById('response-container');

    form.addEventListener('submit', function (event) {
        event.preventDefault();

        const formData = new FormData(form);

        fetch('/cgi/post/form/post.py', {
            method: 'POST',
            body: formData
        })
            .then(response => response.text())
            .then(data => {
                responseContainer.innerHTML = data;
            })
            .catch(error => {
                console.error('Error:', error);
                responseContainer.innerHTML = '<h2>Something went wrong. Please try again later.</h2>';
            });
    });
});
