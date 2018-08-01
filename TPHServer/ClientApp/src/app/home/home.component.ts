import { Component, OnInit, AfterViewInit, Input } from '@angular/core';
import { FormControl } from '@angular/forms';
import { Http, Response } from '@angular/http';
import 'rxjs/add/operator/map';
import { DatePipe } from '@angular/common';

import { Chart } from 'chart.js';
import { ITPHReading, IAggregateRecord, IDevice } from '../../models/iModels';
import { forEach } from '@angular/router/src/utils/collection';
import { DataSource } from '@angular/cdk/collections';
@Component({
  selector: 'home',
  templateUrl: './home.component.html'
})
export class HomeComponent implements OnInit, AfterViewInit {
  baseURL: string = 'api/values/';
  displayType: string = "h";
  gridType: string = "";
  readings: ITPHReading[] = [];
  aggregates: IAggregateRecord[] = [];
  devices: IDevice[] = [];
  device: IDevice;
  busy: boolean = false;
  readingDates: Date[];
  startDate = new FormControl(new Date());
  finishDate = new FormControl(new Date());
  getAll: boolean = false;
  displayAllType: string = "t";
  allAggregates: IAggregateRecord[][] = [];
  selected = 'option2';

  lineChartData: Array<any>;
  standardChartData: Array<any> = [
    { label: 'Temperature', data: [], borderColor: "red", fill: false },
    { label: 'Pressure', data: [], borderColor: "blue", fill: false },
    { label: 'Humidity', data: [], borderColor: "green", fill: false }
  ]
  lineChartOptions: any = {
    scales: {
      xAxes: [{
        type: 'time',
        time: {
          unit: 'day',
          displayFormats: {
            day: 'MMM DD',
            hour: 'MM-DD:HH',
            month: 'YY MMM',

          }
        }
      }]
    },
    tooltips: {
      mode: 'index',
      callbacks: {
        title: function (tooltipItem: any, data: any) {
          var date = new Date(tooltipItem[0].xLabel.substr(0, 10) + " " + tooltipItem[0].xLabel.substr(11, 8) + " UTC");
          return date.toLocaleString();
        }
      }

    }

  };
  public lineChartColors: Array<any> = [
    {
      backgroundColor: 'green',
      borderColor: 'green',
      pointBackgroundColor: 'green',
      pointBorderColor: '#fff',
      pointHoverBackgroundColor: '#fff',
      pointHoverBorderColor: 'rgba(148,159,177,0.8)',
      fill: false
    },
    {
      backgroundColor: 'blue',
      borderColor: 'blue',
      pointBackgroundColor: 'blue',
      pointBorderColor: '#fff',
      pointHoverBackgroundColor: '#fff',
      pointHoverBorderColor: 'rgba(77,83,96,1)',
      fill: false
    },
    {
      backgroundColor: 'yellow',
      borderColor: 'yellow',
      pointBackgroundColor: 'yellow',
      pointBorderColor: '#fff',
      pointHoverBackgroundColor: '#fff',
      pointHoverBorderColor: 'rgba(148,159,177,0.8)',
      fill: false
    }
  ];
  public lineChartLegend: boolean = true;
  public lineChartType: string = 'line';
  constructor(private http: Http, private datePipe: DatePipe) {
  }
  ngOnInit(): void {
    let today = new Date();
    let weekAgo = new Date();
    weekAgo.setDate(today.getDate() - 7);
    this.finishDate.setValue(today);
    this.startDate.setValue(weekAgo);

  }
  ngAfterViewInit() {
    this.subscribeDevices();
  }

  updateReadings() {
    this.busy = true;
    this.subscribeData(this.device.deviceId);
  }
  getData(deviceId: string) {
    var url = this.baseURL;
    var DeviceId = deviceId;
    if (this.getAll) { DeviceId = "ALL"; }
    switch (this.displayType) {
      case 'h':
        url += 'aggregate/' + DeviceId + '/h/' + this.datePipe.transform(this.startDate.value, "yyMMdd") + '00/' + this.datePipe.transform(this.finishDate.value, "yyMMdd") + '23';
        break;
      case 'd':
        url += 'aggregate/' + DeviceId + '/d/' + this.datePipe.transform(this.startDate.value, "yyMMdd") + '/' + this.datePipe.transform(this.finishDate.value, "yyMMdd");
        break;
      case 'm':
        url += 'aggregate/' + DeviceId + '/m/' + this.datePipe.transform(this.startDate.value, "yyMM") + '/' + this.datePipe.transform(this.finishDate.value, "yyMM");
        break;
      default:
        //url += deviceId + '/' + this.datePipe.transform(this.start, "yyMMdd") + '00/' + this.datePipe.transform(this.finish, "yyMMdd") + '23';
        url += deviceId;

    }
    return this.http.get(url)
      .map((res: Response) => res.json());
  }
  subscribeData(deviceId: string) {
    this.getData(deviceId).subscribe(data => {
      //free up memory.
      this.readingDates = [];
      this.allAggregates = [];
      this.aggregates = [];
      this.readings = [];
      if (this.displayType == '') {
        //device readings
        this.readings = data;
        var goodReadings = this.readings.filter(d => d.temperature > -10.00);
        this.lineChartData = this.standardChartData;
        this.lineChartData[0].data = goodReadings.map(d => d.temperature);
        this.lineChartData[1].data = goodReadings.map(d => d.pressure);
        this.lineChartData[2].data = goodReadings.map(d => d.humidity);
        this.readingDates = goodReadings.map(d => d.readAt);
        this.aggregates = [];
        this.allAggregates = [];
        this.readings.sort(function (a, b) {
          var key2 = a.readAt;
          var key1 = b.readAt;

          if (key1 < key2) {
            return -1;
          } else if (key1 == key2) {
            return 0;
          } else {
            return 1;
          }
        });
        console.log(this.readings);


      } else {
        //all devices aggregate readings
        this.aggregates = data;
        this.readings = [];
        if (this.getAll) {
          this.allAggregates = this.normalizeAggregates();
          this.changedisplayAllType(this.displayAllType);
        } else {
          //single device aggregate readings
          this.lineChartData = this.standardChartData;
          this.lineChartData[0].data = this.aggregates.map(d => d.temperature.total / d.temperature.count);
          this.lineChartData[1].data = this.aggregates.map(d => d.pressure.total / d.pressure.count);
          this.lineChartData[2].data = this.aggregates.map(d => d.humidity.total / d.humidity.count);
          this.readingDates = this.getReadingDates(this.aggregates.map(d => d.timeRange));
        }
      }
      this.lineChartOptions.scales.xAxes[0].time.unit = this.getTimeUnit();
      this.gridType = this.displayType;
      this.busy = false;
    });
  }
  getDevices() {
    return this.http.get(this.baseURL + 'account/Temp')
      .map((res: Response) => res.json());
  }
  subscribeDevices() {
    this.getDevices().subscribe(data => {
      this.devices = data;
      this.device = this.devices[0];
      this.updateReadings();

    });
  }
  chartOptions = {
    responsive: true    // THIS WILL MAKE THE CHART RESPONSIVE (VISIBLE IN ANY DEVICE).
  }
  normalizeAggregates(): IAggregateRecord[][] {
    var uniqueDates: string[] = [];
    this.aggregates.map(r => { if (uniqueDates.indexOf(r.timeRange) === -1) { uniqueDates.push(r.timeRange); } });
    uniqueDates.sort((a, b) => a.localeCompare(b));

    var uniqueIds: string[] = [];
    this.aggregates.map(r => { if (uniqueIds.indexOf(r.id) === -1) { uniqueIds.push(r.id); } });


    var normalizedDevices: IAggregateRecord[][] = [];
    var x: IAggregateRecord[] = [];
    for (var i = 0; i < uniqueIds.length; i++) {
      var deviceRecords = this.aggregates.filter(a => a.id == uniqueIds[i]);
      var normalizedDeviceRecords = [];
      for (var j = 0; j < uniqueDates.length; j++) {
        var z = deviceRecords.findIndex(d => d.timeRange == uniqueDates[j]);
        if (z == -1) {
          normalizedDeviceRecords[j] = {
            id: deviceRecords[0].id,
            timeRange: uniqueDates[i],
            temperature: null,
            pressure: null,
            humidity: null
          };
        } else {
          normalizedDeviceRecords[j] = deviceRecords[z];
        }
      }
      normalizedDevices.push(normalizedDeviceRecords);
    }
    this.readingDates = this.getReadingDates(uniqueDates);
    return normalizedDevices;
  }
  getReadingDates(strDates: string[]): Date[] {
    var result: Date[] = [];
    switch (this.displayType) {
      case 'h':
        result = strDates.map(d => new Date(+d.substr(0, 2) + 2000, +d.substr(2, 2) - 1, +d.substr(4, 2), +d.substr(6, 2)));
        break;
      case 'm':
        result = strDates.map(d => new Date(+d.substr(0, 2) + 2000, +d.substr(2, 2) - 1, 1));
        break;

      default:
        result = strDates.map(d => new Date(+d.substr(0, 2) + 2000, +d.substr(2, 2) - 1, +d.substr(4, 2)));

    }
    return result;
  }
  getTimeUnit(): string {
    var unit = "";
    switch (this.displayType) {
      case 'h':
        unit = "hour";
        break;
      case 'm':
        unit = "month";
        break;
      default:
        unit = "day";

    }
    return unit;
  }
  changedisplayAllType(type: string): void {
    this.displayAllType = type;
    this.lineChartData = [];
    var colors: string[] = ['green', 'darkblue', 'yellow', 'red', 'cyan', 'blue', 'lightblue', 'purple', 'lime', 'magenta', 'white', 'silver', 'gray', 'black', 'orange', 'brown', 'maroon', 'olive'];
    for (var i = 0; i < this.allAggregates.length; i++) {
      var deviceData: number[] = [];
      switch (this.displayAllType) {
        case "p":
          deviceData = this.allAggregates[i].map(d => d.pressure == null ? null : d.pressure.total / d.pressure.count);
          break;
        case "h":
          deviceData = this.allAggregates[i].map(d => d.humidity == null ? null : d.humidity.total / d.humidity.count);
          break;
        default:
          deviceData = this.allAggregates[i].map(d => d.temperature == null ? null : d.temperature.total / d.temperature.count);
          break;
      }
      this.lineChartData.push({ label: this.getText(this.allAggregates[i][0].id), data: deviceData, fill: false, color: colors[i], borderColor: colors[i], backgroundColor: colors[i] });

    }
  }
  getText(id: string): string {
    var deviceId = id.slice(0, -1);
    var device = this.devices.filter(d => d.deviceId == deviceId);
    return device[0].location;
  }
}

