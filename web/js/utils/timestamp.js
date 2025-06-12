// web/js/utils/timestamp.js
let currentTimestamp = Date.now();

export function getNextTimestamp() {
    const newTimestamp = Date.now();
    if (newTimestamp <= currentTimestamp) {
        currentTimestamp++;
    } else {
        currentTimestamp = newTimestamp;
    }
    return currentTimestamp;
}