/* ============================================================
   matt.bonis — 3D game integration
   using Three.js
   ============================================================ */

(() => {
  // Check if we are on a desktop or wide enough screen for the full experience.
  // Optional, but usually good for 3D backgrounds.
  const canvasContainer = document.getElementById('game-canvas');
  if (!canvasContainer || !window.THREE) return;

  // 1. Setup Scene, Camera, Renderer
  const scene = new THREE.Scene();
  scene.fog = new THREE.FogExp2(0x0b0f14, 0.05); // Match background color

  const camera = new THREE.PerspectiveCamera(75, window.innerWidth / window.innerHeight, 0.1, 1000);
  camera.position.set(0, 5, 10);
  camera.lookAt(0, 0, -10);

  const renderer = new THREE.WebGLRenderer({ alpha: true, antialias: true });
  renderer.setSize(window.innerWidth, window.innerHeight);
  renderer.setPixelRatio(window.devicePixelRatio);
  canvasContainer.appendChild(renderer.domElement);

  // 2. Create the "Character" (a neon green cube for now)
  const charGeometry = new THREE.BoxGeometry(1, 1, 1);
  const charMaterial = new THREE.MeshBasicMaterial({ color: 0x00ff9c, wireframe: true });
  const character = new THREE.Mesh(charGeometry, charMaterial);
  character.position.set(0, 0.5, 5);
  scene.add(character);

  // 3. Create the "Path" (a grid helper and a glowing line)
  const gridHelper = new THREE.GridHelper(200, 100, 0x1f2a37, 0x131b25);
  gridHelper.position.y = -0.01; // Slightly below character
  scene.add(gridHelper);

  // Expose these for the scroll animation to use later
  window.gameScene = scene;
  window.gameCamera = camera;
  window.gameCharacter = character;
  window.gameRenderer = renderer;

  // Handle Resize
  window.addEventListener('resize', () => {
    camera.aspect = window.innerWidth / window.innerHeight;
    camera.updateProjectionMatrix();
    renderer.setSize(window.innerWidth, window.innerHeight);
  });

  // Animation Loop
  function animate() {
    requestAnimationFrame(animate);

    // Add some passive animation to the character
    character.rotation.x += 0.01;
    character.rotation.y += 0.01;

    renderer.render(scene, camera);
  }
  animate();
})();

  // --- Scroll Logic ---
  const projectsSection = document.getElementById('projects');
  const gameCards = document.querySelectorAll('.game-card');

  if (projectsSection && gameCards.length > 0) {
      window.addEventListener('scroll', () => {
          // Calculate how far we've scrolled within the #projects section
          const rect = projectsSection.getBoundingClientRect();
          const windowHeight = window.innerHeight;

          // rect.top is the distance from viewport top to section top.
          // When rect.top == windowHeight, section is just about to come into view from bottom.
          // When rect.bottom == 0, section is just leaving view from top.
          // Let's use a percentage of scroll through the section itself.

          const sectionHeight = projectsSection.offsetHeight;
          const scrollableDistance = sectionHeight; // We can adjust this if needed

          // distance scrolled past the top of the projects section:
          const scrolledPastTop = window.scrollY - projectsSection.offsetTop + windowHeight/2;

          let progress = 0;
          if (scrolledPastTop > 0) {
             progress = (scrolledPastTop / sectionHeight) * 100;
          }
          if (progress < 0) progress = 0;
          if (progress > 100) progress = 100;

          // Move the character forward based on progress
          // Let's say Z goes from 5 down to -100
          const zStart = 5;
          const zEnd = -100;
          const newZ = zStart + (progress / 100) * (zEnd - zStart);

          if (window.gameCharacter) {
              window.gameCharacter.position.z = newZ;
          }
          if (window.gameCamera) {
              window.gameCamera.position.z = newZ + 5; // Camera follows behind
          }

          // Unlock cards based on progress
          gameCards.forEach(card => {
              const threshold = parseInt(card.getAttribute('data-threshold') || '100', 10);
              if (progress >= threshold) {
                  card.classList.add('unlocked');
              } else {
                  card.classList.remove('unlocked');
              }
          });
      }, { passive: true });
  }
