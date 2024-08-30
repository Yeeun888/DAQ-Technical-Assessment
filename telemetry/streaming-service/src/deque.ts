import { VehicleData } from './server'

export class VDDeque { 
    deque: VehicleData[];
    total: number;

    constructor() { 
        this.deque = []; 
        this.total = 0;
    } 
  
    addFront(element: VehicleData) { 
        this.deque.unshift(element); 
    } 
  
    addNewData(element: VehicleData) { 
        //Push element and make sure is in safe operating range. If not add to array
        this.deque.push(element); 
        if(element.battery_temperature >= 20 && element.battery_temperature <= 80) {
            //Safe operating tempearture
        } else {
            //Unsafe operating temperature 
            this.total += 1;
        }

        //Check if all other elements from dequeue is above time
        //Make sure element exists
        if(!this.isEmpty()) {
            //Front element must be more than 5000ms behind current
            while(element.timestamp - 5000 > this.getFront().timestamp) {
                let temporary = this.removeFront();
                //Only substracts when it is 0
                if(temporary !== null && temporary !== undefined) {
                    this.total -= ((temporary.battery_temperature >= 20 && temporary.battery_temperature <= 80) ? 0 : 1);
                }
            }
        } 
    } 

    lastFiveSecondUnsafeTemperature(): number {
        return this.total
    }
  
    removeFront(): VehicleData | null | undefined { 
        if (!this.isEmpty()) { 
            return this.deque.shift(); 
        }
        return null;
    } 
  
    removeRear() { 
        if (!this.isEmpty()) { 
            return this.deque.pop(); 
        } 
        return null; 
    } 
 
    //No empty checking
    getFront() { 
        return this.deque[0]; 
    } 
  
    //No empty checking
    getRear() { 
        return this.deque[this.size() - 1]; 
    } 
  
    isEmpty() { 
        return this.deque.length === 0; 
    } 
  
    size() { 
        return this.deque.length; 
    } 

    // DEBUGtemperature() {
    //     console.log("Current Log:");
    //     for(const V of this.deque) {
    //         if(V.battery_temperature >= 20 && V.battery_temperature <= 80) {
    //             console.log("%d %f", V.timestamp, V.battery_temperature);
    //         } else {
    //             console.log("%d %f <--- !!!!", V.timestamp, V.battery_temperature);
    //         }
    //     }
    // }
} 
