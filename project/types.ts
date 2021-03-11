export interface LightState {
  led_on: boolean;
}

export interface LightMqttSettings { 
  unique_id : string;
  name: string;
  mqtt_path : string;
}

export interface TransactionState {
  txn_state: string;
  txn_amt:number;
}
