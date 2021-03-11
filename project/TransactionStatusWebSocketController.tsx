import React, { Component } from 'react';
import { ValidatorForm,TextValidator } from 'react-material-ui-form-validator';

import { Typography, Box, Switch } from '@material-ui/core';
import { WEB_SOCKET_ROOT } from '../api';
import { WebSocketControllerProps, WebSocketFormLoader, WebSocketFormProps, webSocketController , FormActions, FormButton} from '../components';
import { SectionContent } from '../components';
import ContactlessIcon from '@material-ui/icons/Contactless';
import Alert from '@material-ui/lab/Alert';
import AlertTitle from '@material-ui/lab/AlertTitle';

import { TransactionState } from './types';

export const LIGHT_SETTINGS_WEBSOCKET_URL = WEB_SOCKET_ROOT + "transtat";

type TransactionStatusWebSocketControllerProps = WebSocketControllerProps<TransactionState>;

class TransactionStatusWebSocketController extends Component<TransactionStatusWebSocketControllerProps> {

  render() {
    return (
      <SectionContent title='Transaction' titleGutter>
        <WebSocketFormLoader
          {...this.props}
          render={props => (
            <TransactionStatusWebSocketControllerForm {...props} />
          )}
        />
      </SectionContent>
    )
  }

}

export default webSocketController(LIGHT_SETTINGS_WEBSOCKET_URL, 100, TransactionStatusWebSocketController);

type TransactionStatusWebSocketControllerFormProps = WebSocketFormProps<TransactionState>;

function TransactionStatusWebSocketControllerForm(props: TransactionStatusWebSocketControllerFormProps) {
  const { data, saveData, setData,handleValueChange } = props;

  const onTxnStart = () => {
   //alert("You are submitting " + data.txn_amt); 
   setData(data, saveData);
   
  }

  return (
    <ValidatorForm onSubmit={saveData}>
      <Box bgcolor="primary.main" color="primary.contrastText" p={2} mt={2} mb={2}>
        <Typography variant="body1">
        Enter amount here and press Submit to start a CloudPoS transaction on the companion Box..
        </Typography>
      </Box>
      <TextValidator
        validators={['required','isNumber']}
        errorMessages={['Amount is required']}
        name="amt"
        label="Amount"
        fullWidth
        variant="outlined"
        value={data?.txn_amt}
        onChange={handleValueChange('txn_amt')}
        margin="normal"
        autoComplete='off'
      />
      <Alert 
      severity="info"
      color="info">{data?.txn_state}</Alert> 
      <FormActions>
        
        <FormButton startIcon={<ContactlessIcon />} variant="contained" color="primary" type="submit">
      
          Confirm
        </FormButton>
      </FormActions>
    </ValidatorForm>
  );
}
