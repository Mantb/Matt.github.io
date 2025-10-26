// JavaScript file (assets/scripts.js)
document.addEventListener('DOMContentLoaded', function() {
    const themeSwitch = document.getElementById('themeSwitch');
    const myName = document.getElementById('myName');

    // Check the theme preference on page load
    if (localStorage.getItem('theme') === 'dark') {
        document.body.classList.add('dark-mode');
        themeSwitch.checked = true;
        myName.classList.add('light-color'); // Add light color class
    }

    // Toggle theme when the switch is clicked
    themeSwitch.addEventListener('change', function() {
        if (this.checked) {
            document.body.classList.add('dark-mode');
            localStorage.setItem('theme', 'dark');
            myName.classList.remove('light-color'); // Remove light color class
        } else {
            document.body.classList.remove('dark-mode');
            localStorage.setItem('theme', 'light');
            myName.classList.add('light-color'); // Add light color class
        }
    });
});
