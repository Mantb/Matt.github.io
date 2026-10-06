/* ============================================================
   matt.bonis — 3D Gamified Portfolio Experience
   using Three.js + GLTFLoader
   ============================================================ */

(() => {
  const canvasContainer = document.getElementById('game-canvas');
  if (!canvasContainer || !window.THREE) return;

  // 1. Setup Scene, Camera, Renderer
  const scene = new THREE.Scene();
  scene.fog = new THREE.FogExp2(0x0b0f14, 0.05);

  const camera = new THREE.PerspectiveCamera(60, window.innerWidth / window.innerHeight, 0.1, 1000);
  // Initial camera position will be updated in the render loop based on the character
  camera.position.set(0, 3, 5);

  const renderer = new THREE.WebGLRenderer({ alpha: true, antialias: true });
  renderer.setSize(window.innerWidth, window.innerHeight);
  renderer.setPixelRatio(window.devicePixelRatio);
  // Enhance shadows for a better look
  renderer.shadowMap.enabled = true;
  renderer.shadowMap.type = THREE.PCFSoftShadowMap;
  canvasContainer.appendChild(renderer.domElement);

  // 2. Lighting
  const ambientLight = new THREE.AmbientLight(0xffffff, 0.4);
  scene.add(ambientLight);

  const dirLight = new THREE.DirectionalLight(0xffffff, 1);
  dirLight.position.set(-10, 20, 10);
  dirLight.castShadow = true;
  dirLight.shadow.camera.top = 20;
  dirLight.shadow.camera.bottom = -20;
  dirLight.shadow.camera.left = -20;
  dirLight.shadow.camera.right = 20;
  dirLight.shadow.mapSize.width = 1024;
  dirLight.shadow.mapSize.height = 1024;
  scene.add(dirLight);

  // 3. Environment & Path (Grid & Curve)
  const gridHelper = new THREE.GridHelper(400, 200, 0x1f2a37, 0x131b25);
  gridHelper.position.y = 0;
  scene.add(gridHelper);

  // Create a winding path for the character to walk along
  const curve = new THREE.CatmullRomCurve3([
    new THREE.Vector3(0, 0, 5),
    new THREE.Vector3(-5, 0, -10),
    new THREE.Vector3(5, 0, -25),
    new THREE.Vector3(-8, 0, -40),
    new THREE.Vector3(0, 0, -60),
    new THREE.Vector3(0, 0, -80)
  ]);

  // Visualize the path as a glowing line
  const points = curve.getPoints(100);
  const pathGeom = new THREE.BufferGeometry().setFromPoints(points);
  const pathMat = new THREE.LineBasicMaterial({ color: 0x00ff9c, linewidth: 2 });
  const pathLine = new THREE.Line(pathGeom, pathMat);
  // We can elevate it slightly to prevent z-fighting with the grid
  pathLine.position.y = 0.02;
  scene.add(pathLine);

  // Expose globals for scroll logic
  window.gameScene = scene;
  window.gameCamera = camera;
  window.gameRenderer = renderer;
  window.gameCurve = curve;
  window.gameMixer = null;
  window.gameActions = {};
  window.gameCharacter = null;
  window.scrollProgress = 0; // 0 to 1

  // 4. Load the Character Model
  const loader = new THREE.GLTFLoader();
  loader.load('https://raw.githubusercontent.com/mrdoob/three.js/master/examples/models/gltf/Soldier.glb', function (gltf) {
    const model = gltf.scene;
    // The soldier model is quite large, let's scale it down
    model.scale.set(1.5, 1.5, 1.5);

    // Enable shadows for the model
    model.traverse((object) => {
      if (object.isMesh) {
        object.castShadow = true;
        object.receiveShadow = true;
        // Adjust materials slightly if desired, but default is usually fine
      }
    });

    // Start position at the beginning of the curve
    const startPoint = curve.getPoint(0);
    model.position.copy(startPoint);

    // Initial look at direction
    const nextPoint = curve.getPoint(0.01);
    model.lookAt(nextPoint);

    scene.add(model);
    window.gameCharacter = model;

    // 5. Setup Animations
    const animations = gltf.animations;
    if (animations && animations.length > 0) {
      const mixer = new THREE.AnimationMixer(model);
      window.gameMixer = mixer;

      // Typical names in Soldier.glb: 'Idle', 'Walk', 'Run'
      // We will grab Idle and Walk
      const idleClip = THREE.AnimationClip.findByName(animations, 'Idle');
      const walkClip = THREE.AnimationClip.findByName(animations, 'Walk');

      if (idleClip) window.gameActions.idle = mixer.clipAction(idleClip);
      if (walkClip) window.gameActions.walk = mixer.clipAction(walkClip);

      // Start by playing Idle
      if (window.gameActions.idle) {
        window.gameActions.idle.play();
      }
    }
  }, undefined, (error) => {
    console.error("An error happened loading the GLTF model:", error);
  });

  // Handle Resize
  window.addEventListener('resize', () => {
    camera.aspect = window.innerWidth / window.innerHeight;
    camera.updateProjectionMatrix();
    renderer.setSize(window.innerWidth, window.innerHeight);
  });

  // Animation Loop Setup
  const clock = new THREE.Clock();

  // Variables for smooth interpolation
  let currentProgress = 0;
  let targetProgress = 0;
  let isScrolling = false;
  let scrollTimeout = null;

  function animate() {
    requestAnimationFrame(animate);

    const delta = clock.getDelta();

    // Update Animation Mixer
    if (window.gameMixer) {
      window.gameMixer.update(delta);
    }

    // Smoothly interpolate current progress towards the target scroll progress
    // This gives a nice eased movement even if scroll events are chunky
    if (window.gameCharacter && window.gameCurve) {
       // Target comes from window.scrollProgress (0 to 1) set by scroll event
       targetProgress = window.scrollProgress;

       // Lerp
       const diff = targetProgress - currentProgress;
       if (Math.abs(diff) > 0.0001) {
           currentProgress += diff * 0.05; // Smoothing factor
       } else {
           currentProgress = targetProgress;
       }

       // Ensure bounds
       currentProgress = Math.max(0, Math.min(1, currentProgress));

       // Calculate position and lookAt on the curve
       const pt = window.gameCurve.getPoint(currentProgress);
       window.gameCharacter.position.copy(pt);

       // Calculate rotation (look ahead)
       const lookAheadProgress = Math.min(1, currentProgress + 0.01);
       if (lookAheadProgress > currentProgress) {
           const lookAtPt = window.gameCurve.getPoint(lookAheadProgress);
           // We want the character to stand upright
           lookAtPt.y = pt.y;
           window.gameCharacter.lookAt(lookAtPt);
       }

       // Third-person camera follow logic
       // Place the camera behind and above the character
       // Get the direction the character is facing
       const charDir = new THREE.Vector3(0, 0, 1);
       charDir.applyQuaternion(window.gameCharacter.quaternion);
       charDir.normalize();

       // Offset: 4 units back, 3 units up
       const camOffset = charDir.clone().multiplyScalar(-5);
       camOffset.y = 3;

       const camPosTarget = pt.clone().add(camOffset);

       // Smooth camera movement
       camera.position.lerp(camPosTarget, 0.1);

       // Look slightly ahead of the character
       const camLookTarget = pt.clone();
       camLookTarget.y += 1.5; // Look at head height
       camera.lookAt(camLookTarget);

       // Handle Animation Switching (Walk vs Idle)
       if (window.gameActions.walk && window.gameActions.idle) {
          if (Math.abs(diff) > 0.001) {
             // Moving
             if (!isScrolling) {
                 isScrolling = true;
                 window.gameActions.walk.reset().play();
                 window.gameActions.idle.crossFadeTo(window.gameActions.walk, 0.2, false);
             }
          } else {
             // Stopped
             if (isScrolling) {
                 isScrolling = false;
                 window.gameActions.idle.reset().play();
                 window.gameActions.walk.crossFadeTo(window.gameActions.idle, 0.2, false);
             }
          }
       }
    }

    renderer.render(scene, camera);
  }

  animate();
})();


  // --- Scroll Logic ---
  const projectsSection = document.getElementById('projects');
  const gameCards = document.querySelectorAll('.game-card');

  if (projectsSection && gameCards.length > 0) {
      window.addEventListener('scroll', () => {
          const rect = projectsSection.getBoundingClientRect();
          const windowHeight = window.innerHeight;
          const sectionHeight = projectsSection.offsetHeight;

          const scrolledPastTop = window.scrollY - projectsSection.offsetTop + windowHeight/2;

          let rawProgress = 0;
          if (scrolledPastTop > 0) {
             rawProgress = (scrolledPastTop / sectionHeight);
          }
          if (rawProgress < 0) rawProgress = 0;
          if (rawProgress > 1) rawProgress = 1;

          // Set the global target for the render loop to interpolate
          window.scrollProgress = rawProgress;

          // Unlock cards based on progress
          // threshold is 0 to 100, rawProgress is 0 to 1
          const pctProgress = rawProgress * 100;
          gameCards.forEach(card => {
              const threshold = parseInt(card.getAttribute('data-threshold') || '100', 10);
              if (pctProgress >= threshold) {
                  card.classList.add('unlocked');
              } else {
                  card.classList.remove('unlocked');
              }
          });
      }, { passive: true });
  }
