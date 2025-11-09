// ===== VARIABLES GLOBALES =====
const ledIndicator = document.getElementById('ledIndicator');
const statusText = document.getElementById('status');
const connectionStatus = document.getElementById('connectionStatus');
const lastUpdateText = document.getElementById('lastUpdate');

// ===== ÉTAT LOCAL DE LA LED =====
let currentLedState = 'off'; // État initial : éteinte

// ===== FONCTION DE MISE À JOUR DE L'HEURE =====
function updateLastUpdateTime() {
  const now = new Date();
  const hours = String(now.getHours()).padStart(2, '0');
  const minutes = String(now.getMinutes()).padStart(2, '0');
  const seconds = String(now.getSeconds()).padStart(2, '0');
  if (lastUpdateText) {
    lastUpdateText.textContent = `Last update: ${hours}:${minutes}:${seconds}`;
  }
}

// ===== FONCTION DE MISE À JOUR DE L'INTERFACE =====
function updateUI(state) {
  currentLedState = state;
  
  // Mettre à jour le texte
  statusText.innerText = 'LED status: ' + state.toUpperCase();
  
  // Mettre à jour l'indicateur visuel
  if (state === 'on') {
    ledIndicator.classList.add('on');
    console.log('💡 Interface mise à jour : LED allumée');
  } else {
    ledIndicator.classList.remove('on');
    console.log('⚫ Interface mise à jour : LED éteinte');
  }
  
  updateLastUpdateTime();
}

// ===== FONCTION DE RAFRAÎCHISSEMENT DU STATUT (optionnel) =====
async function refreshStatus() {
  console.log('🔄 Vérification de l\'état de l\'ESP32...');
  
  try {
    const res = await fetch('/led', {
      method: 'GET',
      cache: 'no-cache',
      headers: {
        'Content-Type': 'application/json'
      }
    });
    
    if (connectionStatus) {
      connectionStatus.classList.remove('disconnected');
    }
    
    if (!res.ok) {
      throw new Error(`HTTP error! status: ${res.status}`);
    }
    
    const text = await res.text();
    const cleanText = text.trim().toLowerCase().replace(/['"]/g, '');
    
    console.log('📥 État ESP32:', cleanText);
    
    // Synchroniser l'interface avec l'état réel de l'ESP32
    updateUI(cleanText);
    
  } catch (error) {
    console.error('❌ Erreur de connexion ESP32:', error.message);
    
    if (connectionStatus) {
      connectionStatus.classList.add('disconnected');
    }
    
    // Ne pas changer l'état affiché en cas d'erreur
    console.log('⚠️  Affichage du dernier état connu');
  }
}

// ===== FONCTION DE CONTRÔLE DE LA LED =====
async function setLED(state) {
  console.log(`🎯 Changement d'état demandé : ${state}`);
  
  // ✨ MISE À JOUR IMMÉDIATE DE L'INTERFACE (sans attendre l'ESP32)
  updateUI(state);
  
  // Sélection de tous les boutons
  const buttons = document.querySelectorAll('button');
  
  // Désactivation temporaire des boutons
  buttons.forEach(btn => {
    btn.disabled = true;
  });

  try {
    // Envoi de la commande à l'ESP32 en arrière-plan
    console.log('📤 Envoi de la commande à l\'ESP32...');
    const response = await fetch('/led', {
      method: 'POST',
      cache: 'no-cache',
      headers: { 
        'Content-Type': 'application/json'
      },
      body: JSON.stringify({ state }),
    });
    
    if (!response.ok) {
      throw new Error(`HTTP error! status: ${response.status}`);
    }
    
    const result = await response.text();
    console.log('✅ Commande envoyée avec succès:', result);
    
    if (connectionStatus) {
      connectionStatus.classList.remove('disconnected');
    }
    
  } catch (error) {
    console.error('❌ Erreur lors de l\'envoi:', error.message);
    
    if (connectionStatus) {
      connectionStatus.classList.add('disconnected');
    }
    
    // L'interface reste dans l'état demandé même en cas d'erreur
    console.log('⚠️  L\'interface affiche l\'état demandé malgré l\'erreur');
    statusText.innerText = 'LED status: ' + state.toUpperCase() + ' (non confirmé)';
  } finally {
    // Réactivation des boutons
    buttons.forEach(btn => {
      btn.disabled = false;
    });
  }
}

// ===== INITIALISATION =====
console.log('🚀 Initialisation de l\'application...');
console.log('📍 Page chargée depuis:', window.location.origin);

// ✨ État initial : LED éteinte
updateUI('off');
console.log('💡 État initial : LED éteinte');

// Vérifier l'état réel de l'ESP32 au chargement (optionnel)
// Décommenter la ligne suivante si vous voulez synchroniser avec l'ESP32 au démarrage
// refreshStatus();

// ✨ OPTIONNEL : Rafraîchissement périodique pour synchroniser avec l'ESP32
// Décommenter les lignes suivantes pour activer la synchronisation automatique
// const intervalId = setInterval(refreshStatus, 30000); // Toutes les 30 secondes
// console.log('⏰ Synchronisation automatique activée (toutes les 30 secondes)');