import React, { Component } from 'react';
import { Redirect, Switch, RouteComponentProps } from 'react-router-dom'

import { Tabs, Tab } from '@material-ui/core';

import { PROJECT_PATH } from '../api';
import { MenuAppBar } from '../components';
import { AuthenticatedRoute } from '../authentication';

import DemoInformation from './CloudPoSInformation';
import TransactionHistoryRestController from './TransactionHistoryRestController';
import TransactionStatusWebSocketController from './TransactionStatusWebSocketController';


class CloudPoS extends Component<RouteComponentProps> {

  handleTabChange = (event: React.ChangeEvent<{}>, path: string) => {
    this.props.history.push(path);
  };

  render() {
    return (
      <MenuAppBar sectionTitle="Cloud PoS Micro">
        <Tabs value={this.props.match.url} onChange={this.handleTabChange} variant="fullWidth">
          <Tab value={`/${PROJECT_PATH}/cloudpos/information`} label="Information" />
          <Tab value={`/${PROJECT_PATH}/cloudpos/rest`} label="Transaction History" />
          <Tab value={`/${PROJECT_PATH}/cloudpos/socket`} label="Transaction" />
        </Tabs>
        <Switch>
          <AuthenticatedRoute exact path={`/${PROJECT_PATH}/cloudpos/information`} component={DemoInformation} />
          <AuthenticatedRoute exact path={`/${PROJECT_PATH}/clouspos/rest`} component={TransactionHistoryRestController} />
          <AuthenticatedRoute exact path={`/${PROJECT_PATH}/cloudpos/socket`} component={TransactionStatusWebSocketController} />
          <Redirect to={`/${PROJECT_PATH}/cloudpos/information`} />
        </Switch>
      </MenuAppBar>
    )
  }

}

export default CloudPoS;
